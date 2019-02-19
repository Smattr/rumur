/* Update: the following is based on a prior pending queue algorithm which used
 * a single-state-per-node linked-list. It has been retained as an interesting
 * large model to run through Rumur.
 *
 * ----
 *
 * Model of the pending queue algorithm in the generated verifier.
 *
 * The generated verifier uses a moderately complex lock-free algorithm for
 * enqueueing and dequeueing to the per-thread pending states queue. The
 * argument for the correctness of this algorithm is non-trivial and quite
 * subtle in places. The model below attempts to capture the abstract logic of
 * this algorithm, with the intent that we can gain some more evidence for its
 * correctness.
 *
 * Where we abstract the implementation:
 *   * In the implementation, each thread maintains the ID of the queue it last
 *     dequeued from (`queue_id`), which is then the first queue it tries to
 *     dequeue from next time. This is an optimisation to reduce scanning
 *     likely-empty queues again. This ID is reset to the thread's own queue
 *     when it enqueues there to terminate work-stealing and switch back to
 *     normal operation. In the model below, we remove this constraint and let
 *     any thread try to dequeue from any queue. This is a safe
 *     over-approximation.
 *   * In the implementation, threads only ever enqueue to their own queue. In
 *     the model we let threads enqueue to arbitrary queues. This is a safe
 *     over-approximation and represents something we might want to consider in
 *     future.
 */
const

  -- number of threads
  THREADS: 2

  /* Number of queues. We use per-thread queues, so this is always the same as
   * the number of threads.
   */
  QUEUES: THREADS

  /* Number of queue nodes available for allocation. Indirectly, this would be
   * something like the available heap memory.
   */
  QUEUE_NODES: 4

  -- a value indicating an invalid queue node
  NULL: -1

type

  -- thread identifier
  thread_id_t: 0 .. THREADS - 1

  -- Queue identifier. Same as thread identifier as we have per-thread queues.
  queue_id_t: 0 .. QUEUES - 1

  /* Queue node identifier. This is an abstraction of what would be virtual
   * address in the real system.
   */
  queue_node_id: 0 .. QUEUE_NODES - 1

  /* As above, but with `-1` to indicate an invalid queue node. Similar to the
   * way we use `NULL` in the real system.
   */
  queue_node_id_opt: -1 .. QUEUE_NODES - 1

  -- the contents of a queue node itself
  queue_node: record
    -- we ignored the state member that is not relevant for verification
    next: queue_node_id_opt
  end

  -- a per-thread queue
  queue: record
    head: queue_node_id_opt -- pointer to the start of the queue
    tail: queue_node_id_opt -- pointer to the end of the queue
  end

  /* A two-pointer structure. This is more general in the implementation, but in
   * the context of this model we only ever need to talk about a struct of two
   * queue pointers, so we make this definition specific.
   */
  double_ptr_t: record
    head: queue_node_id_opt
    tail: queue_node_id_opt
  end

  -- local state used during enqueue()
  enqueue_state: record
    ends:       double_ptr_t
    ends_check: double_ptr_t
    n:          queue_node_id_opt
    new:        double_ptr_t
    queue_id:   queue_id_t
  end

  -- local state used during dequeue()
  dequeue_state: record
    attempts:   0 .. THREADS
    queue_id:   queue_id_t
    ends:       double_ptr_t
    ends_check: double_ptr_t
    new:        double_ptr_t
  end

  -- local state used during reclaim()
  reclaim_state: record
    i:        0 .. THREADS
    j:        0 .. THREADS
    conflict: boolean
  end

  -- program counter values
  label_t: enum {

    -- not running any operation
    IDLE,

    -- running enqueue()
    ENQUEUE_BLOCK_2,
    ENQUEUE_BLOCK_3,
    ENQUEUE_BLOCK_4,
    ENQUEUE_BLOCK_5,
    ENQUEUE_BLOCK_6,
    ENQUEUE_BLOCK_7,
    ENQUEUE_BLOCK_8,
    ENQUEUE_BLOCK_9,
    ENQUEUE_BLOCK_10,
    ENQUEUE_BLOCK_11,

    -- running dequeue()
    DEQUEUE_BLOCK_2,
    DEQUEUE_BLOCK_3,
    DEQUEUE_BLOCK_4,
    DEQUEUE_BLOCK_5,
    DEQUEUE_BLOCK_6,
    DEQUEUE_BLOCK_7,
    DEQUEUE_BLOCK_8,

    -- running reclaim()
    RECLAIM_BLOCK_1,
    RECLAIM_BLOCK_2,
    RECLAIM_BLOCK_3,
    RECLAIM_BLOCK_4,
    RECLAIM_BLOCK_5

  }

  -- thread-local state
  thread_local: record
    pc:            label_t           -- program counter
    head:          queue_node_id_opt -- queue head being currently examined
    tail:          queue_node_id_opt -- queue tail being currently examined

    enqueue_locals: enqueue_state    -- state used during enqueue()
    dequeue_locals: dequeue_state    -- state used during dequeue()
    reclaim_locals: reclaim_state    -- state used during reclaim()

    deferred:       array [0 .. THREADS - 1] of queue_node_id_opt
                                     -- deferred pointers to free used in reclaim()
  end

var

  /* Globally addressable queue nodes. You can think of this as memory indexed
   * by virtual address.
   */
  queue_nodes: array [queue_node_id] of queue_node

  -- IDs of free queue nodes from which malloc allocates
  freelist: array [queue_node_id] of boolean

  -- the pending queues for each thread
  q: array [queue_id_t] of queue

  -- thread-local states
  thread_locals: array [thread_id_t] of thread_local

  -- hazarded pointers
  hazarded: array [thread_id_t] of queue_node_id_opt

function malloc(): queue_node_id_opt; begin

  -- try to find a free queue node
  /* XXX: awkward hack here where we need to loop over queue_node_id_opt instead
   * of queue_node_id because Murphi doesn't like us indexing freelist with a
   * value not of its index type.
   */
  for i: queue_node_id_opt do
    if i != NULL then
      if freelist[i] then
        freelist[i] := false;
        return i;
      end;
    end;
  end;

  -- all queue nodes are in-use (allocated)
  return NULL;

end

procedure free(p: queue_node_id_opt); begin

  -- allow callers to free NULL as a no-op
  if p = NULL then
    return;
  end;

  assert !freelist[p] "freeing a queue node that was not in use";

  freelist[p] := true;

end

-- is the given queue node in the given queue?
function in_queue(qid: queue_id_t; qnid: queue_node_id): boolean;
  var i: queue_node_id_opt
begin

  -- follow the linked-list of queue nodes, looking for our target
  i := q[qid].head;
  while i != NULL do
    if i = qnid then
      return true;
    end;
    i := queue_nodes[i].next;
  end;

  -- we didn't find it
  return false;
end

function double_ptr_make(head: queue_node_id_opt; tail: queue_node_id_opt): double_ptr_t;
var
  tmp: double_ptr_t
begin
  tmp.head := head;
  tmp.tail := tail;
  return tmp;
end

procedure hazard(thread_id: thread_id_t; p: queue_node_id); begin
  assert hazarded[thread_id] = NULL "hazarding multiple pointers at once";

  hazarded[thread_id] := p;
end

procedure unhazard(thread_id: thread_id_t; p: queue_node_id); begin
  assert hazarded[thread_id] != NULL
    "unhazarding a pointer when none are hazarded";

  assert hazarded[thread_id] = p
    "unhazarding a pointer that differs from the one hazarded";

  hazarded[thread_id] := NULL;
end

procedure goto_idle(thread_id: thread_id_t); begin

  -- blank all function-local state
  undefine thread_locals[thread_id].enqueue_locals;
  undefine thread_locals[thread_id].dequeue_locals;
  undefine thread_locals[thread_id].reclaim_locals;

  thread_locals[thread_id].pc := IDLE;
end

startstate begin

  -- mark all queue nodes as unallocated
  for qnid: queue_node_id do
    freelist[qnid] := true;
  end;

  -- set all queues as empty
  for qid: queue_id_t do
    q[qid].head := NULL;
    q[qid].tail := NULL;
  end;

  -- reset all threads
  for thread_id: thread_id_t do
    goto_idle(thread_id);
  end;

  -- we start with no deferred deallocations
  for thread_id: thread_id_t do
    for i: 0 .. THREADS - 1 do
      thread_locals[thread_id].deferred[i] := NULL;
    end;
  end;

  -- we start with no hazarded pointers
  for i: thread_id_t do
    hazarded[i] := NULL;
  end;

end

ruleset thread_id: thread_id_t do
  alias pc: thread_locals[thread_id].pc do

    ruleset queue_id: queue_id_t do
      alias
        n: thread_locals[thread_id].enqueue_locals.n
      do
        rule "enqueue start"
          pc = IDLE ==>
        begin
          thread_locals[thread_id].enqueue_locals.queue_id := queue_id;

          n := malloc();
          if n = NULL then
            goto_idle(thread_id);
            return;
          end;

          queue_nodes[n].next := NULL;

          pc := ENQUEUE_BLOCK_2;
        end
      end
    end

    alias
      ends:       thread_locals[thread_id].enqueue_locals.ends
      ends_check: thread_locals[thread_id].enqueue_locals.ends_check
      head:       thread_locals[thread_id].enqueue_locals.ends.head
      n:          thread_locals[thread_id].enqueue_locals.n
      new:        thread_locals[thread_id].enqueue_locals.new
      queue_id:   thread_locals[thread_id].enqueue_locals.queue_id
      tail:       thread_locals[thread_id].enqueue_locals.ends.tail
    do

      rule "enqueue block 2"
        pc = ENQUEUE_BLOCK_2 ==>
      begin
        ends := q[queue_id];
        pc := ENQUEUE_BLOCK_3;
      end

      rule "enqueue block 3"
        pc = ENQUEUE_BLOCK_3 ==>
      begin
        if tail = NULL then
          assert head = NULL "tail of queue null while head is non-null";

          new := double_ptr_make(n, n);

          if q[queue_id] = ends then
            q[queue_id] := new;
            goto_idle(thread_id);
          else
            ends := q[queue_id];
            -- intentional lack of pc change ("goto retry")
          end;
        else
          hazard(thread_id, tail);
          pc := ENQUEUE_BLOCK_4;
        end;
      end

      rule "enqueue block 4"
        pc = ENQUEUE_BLOCK_4 ==>
      begin
        ends_check := q[queue_id];
        if ends != ends_check then
          pc := ENQUEUE_BLOCK_5;
        else
          pc := ENQUEUE_BLOCK_6;
        end;
      end

      rule "enqueue block 5"
        pc = ENQUEUE_BLOCK_5 ==>
      begin
        unhazard(thread_id, tail);
        ends := ends_check;
        undefine ends_check;
        pc := ENQUEUE_BLOCK_3;
      end

      rule "enqueue block 6"
        pc = ENQUEUE_BLOCK_6 ==>
      begin
        if queue_nodes[tail].next = NULL then
          queue_nodes[tail].next := n;
          pc := ENQUEUE_BLOCK_8;
        else
          pc := ENQUEUE_BLOCK_7;
        end;
      end

      rule "enqueue block 7"
        pc = ENQUEUE_BLOCK_7 ==>
      begin
        unhazard(thread_id, tail);
        pc := ENQUEUE_BLOCK_3;
      end

      rule "enqueue block 8"
        pc = ENQUEUE_BLOCK_8 ==>
      begin
        new := double_ptr_make(head, n);

        if q[queue_id] = ends then
          q[queue_id] := new;
          pc := ENQUEUE_BLOCK_11;
        else
          -- use `ends_check` instead of `old` to save a state variable
          ends_check := q[queue_id];
          pc := ENQUEUE_BLOCK_9;
        end;
      end

      rule "enqueue block 9"
        pc = ENQUEUE_BLOCK_9 ==>
      begin
        assert queue_nodes[tail].next = n "undo of write to tail->next failed";

        queue_nodes[tail].next := NULL;
        pc := ENQUEUE_BLOCK_10;
      end

      rule "enqueue block 10"
        pc = ENQUEUE_BLOCK_10 ==>
      begin
        -- remember, using `ends_check` here where the implementation uses `old`
        unhazard(thread_id, tail);
        ends := ends_check;
        undefine ends_check;
        pc := ENQUEUE_BLOCK_3;
      end

      rule "enqueue block 11"
        pc = ENQUEUE_BLOCK_11 ==>
      begin
        unhazard(thread_id, tail);
        goto_idle(thread_id);
      end
    end

    ruleset queue_id: queue_id_t do
      rule "dequeue start"
        thread_locals[thread_id].pc = IDLE ==>
      begin
        thread_locals[thread_id].dequeue_locals.attempts := 0;
        thread_locals[thread_id].dequeue_locals.queue_id := queue_id;

        thread_locals[thread_id].dequeue_locals.ends := q[queue_id];

        thread_locals[thread_id].pc := DEQUEUE_BLOCK_3;
      end
    end

    alias
      ends:       thread_locals[thread_id].dequeue_locals.ends
      head:       thread_locals[thread_id].dequeue_locals.ends.head
      tail:       thread_locals[thread_id].dequeue_locals.ends.tail
      queue_id:   thread_locals[thread_id].dequeue_locals.queue_id
      attempts:   thread_locals[thread_id].dequeue_locals.attempts
      ends_check: thread_locals[thread_id].dequeue_locals.ends_check
      new:        thread_locals[thread_id].dequeue_locals.new
    do

      rule "dequeue block 2"
        pc = DEQUEUE_BLOCK_2 ==>
      begin
        ends := q[queue_id];
        pc := DEQUEUE_BLOCK_3;
      end

      rule "dequeue block 3"
        pc = DEQUEUE_BLOCK_3 ==>
      begin
        if head != NULL then
          hazard(thread_id, head);
          pc := DEQUEUE_BLOCK_4;
        else
          queue_id := (queue_id + 1) % QUEUES;
          attempts := attempts + 1;
          if attempts < QUEUES then
            pc := DEQUEUE_BLOCK_2;
          else
            goto_idle(thread_id);
          end;
        end;
      end

      rule "dequeue block 4"
        pc = DEQUEUE_BLOCK_4 ==>
      begin
        ends_check := q[queue_id];

        if ends != ends_check then
          pc := DEQUEUE_BLOCK_5;
        else
          undefine ends_check;
          pc := DEQUEUE_BLOCK_6;
        end;
      end

      rule "dequeue block 5"
        pc = DEQUEUE_BLOCK_5 ==>
      begin
        unhazard(thread_id, head);

        ends := ends_check;
        undefine ends_check;

        pc := DEQUEUE_BLOCK_2;
      end

      rule "dequeue block 6"
        pc = DEQUEUE_BLOCK_6 ==>
      begin
        if head = tail then
          new := double_ptr_make(NULL, NULL);
        else
          new := double_ptr_make(queue_nodes[head].next, tail);
        end;

        pc := DEQUEUE_BLOCK_7;
      end

      rule "dequeue block 7"
        pc = DEQUEUE_BLOCK_7 ==>
      begin
        if q[queue_id] = ends then
          q[queue_id] := new;
          undefine tail;
          undefine new;
          pc := DEQUEUE_BLOCK_8;
        else
          undefine new;
          ends_check := q[queue_id];
          pc := DEQUEUE_BLOCK_5;
        end;
      end

      rule "dequeue block 8"
        pc = DEQUEUE_BLOCK_8 ==>
      begin
        unhazard(thread_id, head);
        pc := RECLAIM_BLOCK_1;
      end
    end

    alias
      head:     thread_locals[thread_id].dequeue_locals.ends.head
      i:        thread_locals[thread_id].reclaim_locals.i
      j:        thread_locals[thread_id].reclaim_locals.j
      conflict: thread_locals[thread_id].reclaim_locals.conflict
      deferred: thread_locals[thread_id].deferred
    do

      rule "reclaim block 1"
        pc = RECLAIM_BLOCK_1 ==>
      begin
        assert head != NULL "reclaiming a null pointer";

        assert hazarded[thread_id] = NULL
          "reclaiming a pointer while holding a hazarded pointer";

        -- compressed loop iterations until we find something interesting
        i := 0;
        while i < THREADS & deferred[i] = NULL do
          i := i + 1;
        end;

        if i < THREADS then
          conflict := false;
          j := 0;
          pc := RECLAIM_BLOCK_2;
        else
          conflict := false;
          i := 0;
          pc := RECLAIM_BLOCK_4;
        end;
      end

      rule "reclaim block 2"
        pc = RECLAIM_BLOCK_2 ==>
      begin
        assert i < THREADS;
        assert deferred[i] != NULL;

        if j = thread_id then
          assert hazarded[j] = NULL;
        elsif deferred[i] = hazarded[j] then
          conflict := true;
          j := THREADS - 1; -- "break"
        end;

        j := j + 1;

        if j = THREADS then
          undefine j;
          pc := RECLAIM_BLOCK_3;
        end;
      end

      rule "reclaim block 3"
        pc = RECLAIM_BLOCK_3 ==>
      begin
        if !conflict then
          free(deferred[i]);
          deferred[i] := NULL;
        end;

        undefine conflict;

        while i < THREADS & deferred[i] = NULL do
          i := i + 1;
        end;

        if i < THREADS then
          conflict := false;
          j := 0;
          pc := RECLAIM_BLOCK_2;
        else
          conflict := false;
          i := 0;
          pc := RECLAIM_BLOCK_4;
        end;
      end

      rule "reclaim block 4"
        pc = RECLAIM_BLOCK_4 ==>
      begin
        assert i < THREADS;

        if i = thread_id then
          assert hazarded[i] = NULL;
        elsif head = hazarded[i] then
          conflict := true;
          i := THREADS - 1; -- "break"
        end;

        i := i + 1;

        if i = THREADS then
          undefine i;
          pc := RECLAIM_BLOCK_5;
        end;
      end

      rule "reclaim block 5"
        pc = RECLAIM_BLOCK_5 ==>
      begin

        if !conflict then
          free(head);
          goto_idle(thread_id);
          return;
        else
          for ii: 0 .. THREADS - 1 do
            if deferred[ii] = NULL then
              deferred[ii] := head;
              goto_idle(thread_id);
              return;
            end;
          end;
        end;

        assert false "deferred more than `THREADS` reclamations";
      end
    end
  end
end

invariant "queue only empty when head and tail agree"
  forall qid: queue_id_t do
    (q[qid].head = NULL -> q[qid].tail = NULL) &
    (q[qid].tail = NULL -> q[qid].head = NULL)
  end

invariant "no memory leaks"
  /* at quiescence, every queue node is either in a queue, in the free list, or
   * in a thread's deferred deallocations list
   */
  exists thread_id: thread_id_t do thread_locals[thread_id].pc != IDLE end |
  forall qnid: queue_node_id do
    exists qid: queue_id_t do in_queue(qid, qnid) end |
    freelist[qnid] |
    exists thread_id: thread_id_t do
      exists i: 0 .. THREADS - 1 do
        thread_locals[thread_id].deferred[i] = qnid
      end
    end
  end

invariant "no double use of nodes"
  -- every queue node in a queue is nowhere else
  forall qnid: queue_node_id do
    forall qid: queue_id_t do
      in_queue(qid, qnid) ->
        (forall qid2: queue_id_t do qid2 = qid | !in_queue(qid2, qnid) end
        & !freelist[qnid]
        & forall thread_id: thread_id_t do
            forall i: 0 .. THREADS - 1 do
              thread_locals[thread_id].deferred[i] != qnid
            end
          end)
    end
  end
