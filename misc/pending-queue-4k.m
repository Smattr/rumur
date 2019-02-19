/* Model of the pending queue algorithm in the generated verifier. This was
 * originally adapted from pending-queue.m.
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
 *   * The queue nodes of the implementation store
 *     `4096 / sizeof(struct state*) - 1` states. In the model, we store fewer
 *     (`STATES_PER_NODE`) to make the model manageable.
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
  QUEUE_NODES: 2

  /* Number of states stored per queue node. In the implementation, this is
   * `4096 / sizeof(struct state*) - 1`, but we abstract this here to a value
   * that should still be reasonable enough to expose any concurrency problems.
   */
  STATES_PER_NODE: 2

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

  -- handle to either a state within a queue node or the node's next pointer
  queue_handle_t: record
    node: queue_node_id_opt
    offset: 0 .. STATES_PER_NODE
  end

  -- the contents of a queue node itself
  queue_node: record
    /* we only model whether a state slot is occupied or not, not what actual
     * state pointer it contains
     */
    s: array[0 .. STATES_PER_NODE - 1] of boolean
    next: queue_node_id_opt
  end

  -- a per-thread queue
  queue: record
    head: queue_handle_t -- handle to the start of the queue
    tail: queue_handle_t -- handle to the end of the queue
  end

  /* A two-pointer structure. This is more general in the implementation, but in
   * the context of this model we only ever need to talk about a struct of two
   * queue handles, so we make this definition specific.
   */
  double_ptr_t: record
    head: queue_handle_t
    tail: queue_handle_t
  end

  -- local state used during enqueue()
  enqueue_state: record
    ends:       double_ptr_t
    ends_check: double_ptr_t
    n:          queue_node_id_opt
    new:        double_ptr_t
    queue_id:   queue_id_t
    new_node:   queue_node_id_opt
    next_tail:  queue_handle_t
  end

  -- local state used during dequeue()
  dequeue_state: record
    attempts:     0 .. THREADS
    queue_id:     queue_id_t
    ends:         double_ptr_t
    ends_check:   double_ptr_t
    new:          double_ptr_t
    new_head:     queue_node_id_opt
    reclaim_call: enum { L1911, L1940 }
    old:          double_ptr_t
    s:            boolean
  end

  -- local state used during reclaim()
  reclaim_state: record
    i:        0 .. THREADS
    j:        0 .. THREADS
    conflict: boolean
  end

  /* Program counter values. Note that some of these suffixed with 'L*' refer to
   * source code lines in ../rumur/resources/header.c at the time of writing.
   * These may have drifted out of sync with changes since then, so if you want
   * to cross reference these you may have to consult the Git commit in which
   * this model was added.
   */
  label_t: enum {

    -- not running any operation
    IDLE,

    -- running enqueue()
    ENQUEUE_L1708,
    ENQUEUE_L1723,
    ENQUEUE_L1726,
    ENQUEUE_L1737,
    ENQUEUE_L1740,
    ENQUEUE_L1751,
    ENQUEUE_L1764,
    ENQUEUE_L1784,
    ENQUEUE_L1787,
    ENQUEUE_L1788,
    ENQUEUE_L1797,
    ENQUEUE_L1813,
    ENQUEUE_L1829,
    ENQUEUE_L1832,
    ENQUEUE_L1840,

    -- running dequeue()
    DEQUEUE_L1859,
    DEQUEUE_L1861,
    DEQUEUE_L1870,
    DEQUEUE_L1873,
    DEQUEUE_L1882,
    DEQUEUE_L1904,
    DEQUEUE_L1908,
    DEQUEUE_L1909,
    DEQUEUE_L1913,
    DEQUEUE_L1919,
    DEQUEUE_L1922,
    DEQUEUE_L1932,
    DEQUEUE_L1939,
    DEQUEUE_L1943,

    -- running reclaim()
    RECLAIM_L1503,
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

function queue_node_new(): queue_node_id_opt; begin

  -- try to find a free queue node
  /* XXX: awkward hack here where we need to loop over queue_node_id_opt instead
   * of queue_node_id because Murphi doesn't like us returning a value of a
   * different type than the function's return type.
   */
  for i: queue_node_id_opt do
    if i != NULL then
      if freelist[i] then
        freelist[i] := false;

        -- memset 0
        for j: 0 .. STATES_PER_NODE - 1 do
          queue_nodes[i].s[j] := false;
        end;
        queue_nodes[i].next := NULL;

        return i;
      end;
    end;
  end;

  -- all queue nodes are in-use (allocated)
  return NULL;

end

procedure queue_node_free(p: queue_node_id_opt); begin

  -- allow callers to free NULL as a no-op
  if p = NULL then
    return;
  end;

  assert !freelist[p] "freeing a queue node that was not in use";

  freelist[p] := true;

end

function queue_handle_from_node_ptr(n: queue_node_id_opt): queue_handle_t;
  var
    tmp: queue_handle_t
begin
  tmp.node := n;
  tmp.offset := 0;
  return tmp;
end

function queue_handle_base(h: queue_handle_t): queue_node_id_opt;
  return h.node;
end

function queue_handle_is_state_pptr(h: queue_handle_t): boolean;
begin
  return h.offset < STATES_PER_NODE;
end

function queue_handle_next(h: queue_handle_t): queue_handle_t;
  var tmp: queue_handle_t
begin
  assert h.offset < STATES_PER_NODE
    "queue_handle_next() called on exhausted queue handle";

  tmp.node := h.node;
  tmp.offset := h.offset + 1;

  return tmp;
end

-- is the given queue node in the given queue?
function in_queue(qid: queue_id_t; qnid: queue_node_id): boolean;
  var i: queue_node_id_opt
begin

  -- follow the linked-list of queue nodes, looking for our target
  i := q[qid].head.node;
  while i != NULL do
    if i = qnid then
      return true;
    end;
    i := queue_nodes[i].next;
  end;

  -- we didn't find it
  return false;
end

function double_ptr_make(head: queue_handle_t; tail: queue_handle_t): double_ptr_t;
  var tmp: double_ptr_t
begin
  tmp.head := head;
  tmp.tail := tail;
  return tmp;
end

procedure hazard(thread_id: thread_id_t; h: queue_handle_t);
  var
    p: queue_node_id_opt
begin
  p := queue_handle_base(h);

  assert hazarded[thread_id] = NULL "hazarding multiple pointers at once";

  hazarded[thread_id] := p;
end

procedure unhazard(thread_id: thread_id_t; h: queue_handle_t);
  var p: queue_node_id_opt
begin
  p := queue_handle_base(h);

  assert hazarded[thread_id] != NULL
    "unhazarding a pointer when none are hazarded";

  assert hazarded[thread_id] = p
    "unhazarding a pointer that differs from the one hazarded";

  hazarded[thread_id] := NULL;
end

procedure return_from_reclaim(thread_id: thread_id_t); begin
  alias reclaim_call: thread_locals[thread_id].dequeue_locals.reclaim_call
        pc: thread_locals[thread_id].pc do

    assert !isundefined(reclaim_call) "unknown reclaim() call label";

    if reclaim_call = L1911 then
      pc := DEQUEUE_L1913;
    else
      pc := DEQUEUE_L1943
    end;

    undefine reclaim_call;
  end;
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
    q[qid].head.node := NULL;
    q[qid].head.offset := 0;
    q[qid].tail.node := NULL;
    q[qid].tail.offset := 0;
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
      rule "enqueue start"
        pc = IDLE ==>
      begin
        thread_locals[thread_id].enqueue_locals.queue_id := queue_id;

        thread_locals[thread_id].enqueue_locals.ends := q[queue_id];
        pc := ENQUEUE_L1708;
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
      new_node:   thread_locals[thread_id].enqueue_locals.new_node
      next_tail:  thread_locals[thread_id].enqueue_locals.next_tail
    do

      rule "header.c:1708"
        pc = ENQUEUE_L1708 ==>
      begin
        if tail.node = NULL then
          assert tail.offset = 0
            "queue_node_offset has non-zero offset with null node";

          assert head.node = NULL "tail of queue 0 while head is non-0";
          assert head.offset = 0
            "queue_node_offset has non-zero offset with null node";

          n := queue_node_new();
          assume n != NULL; -- avoid dealing with OOM
          queue_nodes[n].s[0] := true;

          new := double_ptr_make(queue_handle_from_node_ptr(n),
                                 queue_handle_from_node_ptr(n));

          pc := ENQUEUE_L1723;
        else
          hazard(thread_id, tail);
          pc := ENQUEUE_L1737;
        end;
      end

      rule "header.c:1723"
        pc = ENQUEUE_L1723 ==>
      begin
        if q[queue_id] = ends then
          q[queue_id] := new;
          goto_idle(thread_id);
        else
          ends := q[queue_id];
          pc := ENQUEUE_L1726;
        end;
      end

      rule "header.c:1726"
        pc = ENQUEUE_L1726 ==>
      begin
        queue_node_free(n);
        undefine n;
        pc := ENQUEUE_L1708; -- "goto retry"
      end

      rule "header.c:1737"
        pc = ENQUEUE_L1737 ==>
      begin
        ends_check := q[queue_id];
        if ends != ends_check then
          pc := ENQUEUE_L1740;
        else
          pc := ENQUEUE_L1751;
        end;
      end

      rule "header.c:1740"
        pc = ENQUEUE_L1740 ==>
      begin
        unhazard(thread_id, tail);
        ends := ends_check;
        undefine ends_check;
        pc := ENQUEUE_L1708; -- "goto retry"
      end

      rule "header.c:1751"
        pc = ENQUEUE_L1751 ==>
      begin
        next_tail := queue_handle_next(tail);

        if queue_handle_is_state_pptr(next_tail) then
          alias target: queue_nodes[next_tail.node].s[next_tail.offset] do
            if !target then
              target := true;
              pc := ENQUEUE_L1797;
            else
              undefine next_tail;
              pc := ENQUEUE_L1764;
            end;
          end;
        else
          new_node := queue_node_new();
          assume new_node != NULL; -- avoid dealing with OOM
          queue_nodes[new_node].s[0] := true;

          pc := ENQUEUE_L1784;
        end;
      end

      rule "header.c:1764"
        pc = ENQUEUE_L1764 ==>
      begin
        unhazard(thread_id, tail);
        pc := ENQUEUE_L1708; -- "goto retry"
      end

      rule "header.c:1784"
        pc = ENQUEUE_L1784 ==>
      begin
        alias target: queue_nodes[next_tail.node].next do
          if target = NULL then
            target := new_node;

            next_tail := queue_handle_from_node_ptr(new_node);
            pc := ENQUEUE_L1797;
          else
            undefine next_tail;
            pc := ENQUEUE_L1787;
          end;
        end;
      end

      rule "header.c:1787"
        pc = ENQUEUE_L1787 ==>
      begin
        queue_node_free(new_node);
        undefine new_node;
        pc := ENQUEUE_L1788;
      end

      rule "header.c:1788"
        pc = ENQUEUE_L1788 ==>
      begin
        unhazard(thread_id, tail);
        pc := ENQUEUE_L1708; -- "goto retry"
      end

      rule "header.c:1797"
        pc = ENQUEUE_L1797 ==>
      begin
        new := double_ptr_make(head, next_tail);

        if q[queue_id] = ends then
          q[queue_id] := new;
          pc := ENQUEUE_L1840;
        else
          -- use `ends_check` instead of `old` to save a state variable
          ends_check := q[queue_id];
          pc := ENQUEUE_L1813;
        end;
      end

      rule "header.c:1813"
        pc = ENQUEUE_L1813 ==>
      begin
        next_tail := queue_handle_next(tail);
        if queue_handle_is_state_pptr(next_tail) then
          alias target: queue_nodes[next_tail.node].s[next_tail.offset] do
            if target then
              target := false;
              pc := ENQUEUE_L1832;
            else
              assert false "undo of write to next_tail failed";
            end;
          end;
        else
          alias target: queue_nodes[next_tail.node].next do
            if target = new_node then
              target := NULL;
              pc := ENQUEUE_L1829;
            else
              assert false "undo of write to next_tail failed";
            end;
          end;
        end;
      end

      rule "header.c:1829"
        pc = ENQUEUE_L1829 ==>
      begin
        queue_node_free(new_node);
        undefine new_node;
        pc := ENQUEUE_L1832;
      end

      rule "header.c:1832"
        pc = ENQUEUE_L1832 ==>
      begin
        unhazard(thread_id, tail);
        ends := ends_check;
        undefine ends_check;
        pc := ENQUEUE_L1708; -- "goto retry"
      end

      rule "header.c:1840"
        pc = ENQUEUE_L1840 ==>
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

        thread_locals[thread_id].pc := DEQUEUE_L1861;
      end
    end

    alias
      ends:         thread_locals[thread_id].dequeue_locals.ends
      head:         thread_locals[thread_id].dequeue_locals.ends.head
      tail:         thread_locals[thread_id].dequeue_locals.ends.tail
      queue_id:     thread_locals[thread_id].dequeue_locals.queue_id
      attempts:     thread_locals[thread_id].dequeue_locals.attempts
      ends_check:   thread_locals[thread_id].dequeue_locals.ends_check
      new:          thread_locals[thread_id].dequeue_locals.new
      new_head:     thread_locals[thread_id].dequeue_locals.new_head
      reclaim_call: thread_locals[thread_id].dequeue_locals.reclaim_call
      old:          thread_locals[thread_id].dequeue_locals.old
      s:            thread_locals[thread_id].dequeue_locals.s
    do

      rule "header.c:1859"
        pc = DEQUEUE_L1859 ==>
      begin
        ends := q[queue_id];
        pc := DEQUEUE_L1861;
      end

      rule "header.c:1861"
        pc = DEQUEUE_L1861 ==>
      begin
        if head.node != NULL then
          hazard(thread_id, head);
          pc := DEQUEUE_L1870;
        else
          queue_id := (queue_id + 1) % QUEUES;
          attempts := attempts + 1;
          if attempts < QUEUES then
            pc := DEQUEUE_L1859;
          else
            goto_idle(thread_id);
          end;
        end;
      end

      rule "header.c:1870"
        pc = DEQUEUE_L1870 ==>
      begin
        ends_check := q[queue_id];

        if ends != ends_check then
          pc := DEQUEUE_L1873;
        else
          undefine ends_check;
          pc := DEQUEUE_L1882;
        end;
      end

      rule "header.c:1873"
        pc = DEQUEUE_L1873 ==>
      begin
        unhazard(thread_id, head);

        ends := ends_check;
        undefine ends_check;

        pc := DEQUEUE_L1861; -- "goto retry"
      end

      rule "header.c:1882"
        pc = DEQUEUE_L1882 ==>
      var
        zero: queue_handle_t
      begin
        if head = tail then
          zero.node := NULL;
          zero.offset := 0;
          new := double_ptr_make(zero, zero);

          pc := DEQUEUE_L1919;
        elsif queue_handle_is_state_pptr(head) then
          new := double_ptr_make(queue_handle_next(head), tail);

          pc := DEQUEUE_L1919;
        else
          new_head := queue_nodes[head.node].next;
          new := double_ptr_make(queue_handle_from_node_ptr(new_head), tail);

          pc := DEQUEUE_L1904;
        end;
      end

      rule "header.c:1904"
        pc = DEQUEUE_L1904 ==>
      begin
        old := q[queue_id];
        if q[queue_id] = ends then
          q[queue_id] := new;
        end;
        pc := DEQUEUE_L1908;
      end

      rule "header.c:1908"
        pc = DEQUEUE_L1908 ==>
      begin
        unhazard(thread_id, head);
        pc := DEQUEUE_L1909;
      end

      rule "header.c:1909"
        pc = DEQUEUE_L1909 ==>
      begin
        if old = ends then
          reclaim_call := L1911;
          pc := RECLAIM_L1503;
        else
          pc := DEQUEUE_L1913;
        end;
      end

      rule "header.c:1913"
        pc = DEQUEUE_L1913 ==>
      begin
        ends := old;
        undefine old;
        pc := DEQUEUE_L1861; -- "goto retry"
      end

      rule "header.c:1919"
        pc = DEQUEUE_L1919 ==>
      begin
        if q[queue_id] = ends then
          q[queue_id] := new;
          undefine new;
          pc := DEQUEUE_L1932;
        else
          old := q[queue_id];
          undefine new;
          pc := DEQUEUE_L1922;
        end;
      end

      rule "header.c:1922"
        pc = DEQUEUE_L1922 ==>
      begin
        unhazard(thread_id, head);
        ends := old;
        undefine old;
        pc := DEQUEUE_L1861; -- "goto retry"
      end

      rule "header.c:1932"
        pc = DEQUEUE_L1932 ==>
      begin
        if queue_handle_is_state_pptr(head) then
          s := true;
        else
          s := false;
        end;

        unhazard(thread_id, head);

        pc := DEQUEUE_L1939;
      end;

      rule "header.c:1939"
        pc = DEQUEUE_L1939 ==>
      begin
        if head = tail | !queue_handle_is_state_pptr(head) then
          reclaim_call := L1940;
          pc := RECLAIM_L1503;
        else
          pc := DEQUEUE_L1943;
        end;
      end

      rule "header.c:1943"
        pc = DEQUEUE_L1943 ==>
      begin
        if !s then
          queue_id := (queue_id + 1) % QUEUES;
          attempts := attempts + 1;
          if attempts < QUEUES then
            pc := DEQUEUE_L1859;
          else
            goto_idle(thread_id);
          end;
        else
          goto_idle(thread_id);
        end;
      end
    end

    alias
      head:         thread_locals[thread_id].dequeue_locals.ends.head
      reclaim_call: thread_locals[thread_id].dequeue_locals.reclaim_call
      i:            thread_locals[thread_id].reclaim_locals.i
      j:            thread_locals[thread_id].reclaim_locals.j
      conflict:     thread_locals[thread_id].reclaim_locals.conflict
      deferred:     thread_locals[thread_id].deferred
    do

      rule "header.c:1503"
        pc = RECLAIM_L1503 ==>
      begin
        assert !isundefined(reclaim_call)
          "no return address when entering reclaim()";

        assert head.node != NULL "reclaiming a null pointer";

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
          queue_node_free(deferred[i]);
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
        elsif head.node = hazarded[i] then
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
          queue_node_free(head.node);
          return_from_reclaim(thread_id);
          return;
        else
          for ii: 0 .. THREADS - 1 do
            if deferred[ii] = NULL then
              deferred[ii] := head.node;
              return_from_reclaim(thread_id);
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
    (q[qid].head.node = NULL -> q[qid].head.offset = 0) &
    (q[qid].head.node = NULL -> q[qid].tail.node = NULL) &
    (q[qid].tail.node = NULL -> q[qid].tail.offset = 0) &
    (q[qid].tail.node = NULL -> q[qid].head.node = NULL)
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
