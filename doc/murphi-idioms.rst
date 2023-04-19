Murphi Idioms
=============
This document works through modelling an example system, demonstrating some
patterns commonly seen in Murphi models.

Example: Multithreaded Counter
------------------------------
Lets take a simple concurrent software program and see how to prove some
properties about it. Our example will use 10 threads to count to 10000:

.. code-block:: c

  #include <assert.h>
  #include <pthread.h>
  #include <stddef.h>

  static int total;

  static void *run(void *ignored) {
    (void)ignored;

    for (size_t i = 0; i < 1000; ++i)
      ++total;

    return NULL;
  }

  int main(void) {

    // start 10 threads to count
    pthread_t threads[10];
    for (size_t i = 0; i < sizeof(threads) / sizeof(threads[0]); ++i) {
      int r = pthread_create(&threads[i], NULL, run, NULL);
      assert(r == 0);
    }

    // wait for them to finish
    for (size_t i = 0; i < sizeof(threads) / sizeof(threads[0]); ++i) {
      void *ignored;
      int r = pthread_join(threads[i], &ignored);
      assert(r == 0);
    }

    // is the count the total we expect?
    assert(total == 10000);

    return 0;
  }

What happens when we run this program?

.. code-block:: bash

  $ gcc -Wall -Wextra -std=c99 -pthread counter.c -o counter -lpthread
  $ ./counter

Everything looks good. But what happens if we run it a few more times?

.. code-block:: bash

  $ ./counter
  $ ./counter
  $ ./counter
  counter: counter.c:33: main: Assertion `total == 10000' failed.
  [1]    2030135 IOT instruction (core dumped)  ./counter

Oh dear, we have a problem. Our program is non-deterministic and sometimes
fails.

If you are experienced with writing multithreaded C, you may already have
spotted the problem. But you have likely also seen more complex systems where
identifying bugs was not so easy. Moreover, not every program is as easy to
write correctness assertions for as this one. Lets see if Rumur can find bugs
for us.

Global State and Thread-local State
-----------------------------------
Concurrent systems typically have some state that is universally visible and
other state that is per-agent. Mapping these concepts onto your implementation
is usually straightforward. Programming languages generally have their own
equivalent of these concepts. For our example, the global is part of global
state while the threads’ loop counters are part of thread-local state:

.. code-block:: murphi

  const INT_MIN: -2147483648;
  const INT_MAX: 2147483647;
  type int: INT_MIN..INT_MAX;

  var total: int;

  const THREADS: 10;
  type thread_id: 0..THREADS-1;

  type tls_t: record
         i: int;
       end;
  var tls: array[thread_id] of tls_t;

Program Counters
----------------
We need some way of keeping track of where a thread’s execution is up to.
Something similar to what line of ``run`` they are at, keeping in mind that it
will not exactly be a line number because C statements are not atomic steps.
That is, the ``run`` function is more precisely something like:

.. code-block:: c

  for (size_t i = 0; i < 1000; ++i) {
    int tmp = total;
    total = tmp + 1;
  }

In this transformation, we have left the thread-local variable ``i`` as written
because it is not visible to other threads, but expanded out the operations on
the global ``total``. (For the purposes of this example, we will ignore that
reading and writing an ``int`` may itself not be atomic.)

A consequence of this expansion is that we need to track the thread-local value
we have upon reading ``total``:

.. code-block:: diff

   record tls_t: record
            i: int;
  +         tmp: int;
          end;

Now lets introduce some labels so we have a way to name execution locations:

.. code-block:: c

  for (size_t i = 0; i < 1000; ++i) {
  LOOP_START:
    int tmp = total;
  READ_TOTAL:
    total = tmp + 1;
  }
  DONE:

Not all systems can be easily transformed like this within the source language;
sometimes the concurrency you wish to model occurs beyond the level of
expressibility of your source language. But this is still the mental elaboration
to keep in mind.

To replicate this in Murphi, we need to add a program counter to the
thread-local state:

.. code-block:: diff

  +type label_t: enum { LOOP_START, READ_TOTAL, DONE }
  +
   record tls_t: record
  +         pc: label_t;
            i: int;
            tmp: int;
          end;

Though we are discussing a software system, most concurrent hardware systems
also have something analogous to the program counter for conceptually tracking
thread-local progress.

Statements as Steps
-------------------
Translating the implementation to a series of rule steps requires thinking about
the granularity of transitions. Rules execute atomically, so the points at which
you wish to allow interleaving of thread steps align with the boundaries between
one rule and the next.

An outer ruleset captures that any thread can try to take a step:

.. code-block:: murphi

  ruleset t: thread_id do
    rule "tmp = total"
        tls[t].pc = LOOP_START
      & tls[t].i < 1000
      ==>
    begin
      tls[t].tmp := total;
      tls[t].pc := READ_TOTAL;
    end

    rule "total = tmp + 1"
        tls[t].pc = READ_TOTAL
      ==>
    begin
      total := tls[t].tmp + 1;
      tls[t].i := tls[t].i + 1;
      tls[t].pc := LOOP_START;
    end

    rule "exit loop"
        tls[t].pc = LOOP_START
      & tls[t].i >= 1000
      ==>
    begin
      tls[t].pc := DONE;
    end
  end

Note how each rule has, as part of its guard, a restriction on the program
counter confining it to an exact location. And each rule has, as its final
action, an update of the program counter that moves to another location.

Initialisation/Reset
--------------------
To know what state the system is in to begin with, we need to define a start
state. In a software system this might be referred to as “initialisation” and in
a hardware system it might be called “coming out of reset.”

We can define a start state to capture static initialisation (the C variable
``total`` is set to 0 on startup) and the threads’ loop initialisation:

.. code-block:: murphi

  startstate begin
    total := 0;
    for t: thread_id do
      tls[t].pc := LOOP_START;
      tls[t].i := 0;
    end;
  end

Note that touching both global and thread-local state in the start state is
typical.

Specifying Correctness
----------------------
We have so far avoided the hardest part of formal modelling: what does “correct”
mean for my system? This is hard because it is inherently system-specific. What
it means for a system to be correct is dependent on its *purpose*. Ideally it
should capture both what the system should do and what it should *not* do.

For this example, we will take the property from the final assertion,
``total == 10000``:

.. code-block:: murphi

  -- if we are done, the counter is at its expected value
  invariant
    forall t: thread_id do tls[t].pc = DONE end
    ->
    total = 10000;

For more sophisticated systems, correctness properties can be arbitrarily
complex and can themselves take significant work to develop.

Abstraction
-----------
We seem to have a full system definition. So what happens when we ask Rumur to
check it?

.. code-block:: bash

  $ rumur-run counter.m
  Generating the checker...
  Compiling the checker...
  Running the checker...
  Memory usage:

      * The size of each state is 713 bits (rounded up to 90 bytes).
      * The size of the hash table is 8192 slots.

  Progress Report:

      …

It runs for a long time and eventually runs out of memory…

So what is Rumur good for if it cannot find our bugs? Well, we asked it to do
something unreasonable. Think about the interleaving of 10 threads across 1000
steps. This is a *lot* of possible executions to explore.

This is where *abstraction* can help.

Firstly, do we really need 1000 loop iterations? This is what the implementation
does, but the uniformity of this program suggests we can find the bug with much
fewer:

.. code-block:: diff

   rule "tmp = total"
       tls[t].pc = LOOP_START
  -  & tls[t].i < 1000
  +  & tls[t].i < 3
     ==>
   begin
     tls[t].tmp := total;
     tls[t].pc := READ_TOTAL;
   end

  …

   rule "exit loop"
       tls[t].pc = LOOP_START
  -  & tls[t].i >= 1000
  +  & tls[t].i >= 3
     ==>
   begin
     tls[t].pc := DONE;
   end

  …

   -- if we are done, the counter is at its expected value
   invariant
     forall t: thread_id do tls[t].pc = DONE end
     ->
  -  total = 10000;
  +  total = 30;

Next, do we really need 10 threads to find this bug? The vast majority of race
conditions can be found with 3 or fewer threads (citation needed…):

.. code-block:: diff

  -const THREADS: 10;
  +const THREADS: 3;
   type thread_id: 0..THREADS-1;

  …

   -- if we are done, the counter is at its expected value
   invariant
     forall t: thread_id do tls[t].pc = DONE end
     ->
  -  total = 30;
  +  total = 9;

When we reattempt checking, Rumur can now find a bug (discussed below). This
section walked through how to abstract a model into something checkable, but
observe that it is somewhat of an art rather than a science. Knowing which
numbers you can decrease without accidentally masking bugs is a skill that comes
with experience.

Interpreting a Counterexample
-----------------------------
After abstraction, Rumur is able to find a bug:

.. code-block:: bash

  $ rumur-run counter.m
  Generating the checker...
  Compiling the checker...
  Running the checker...
  Memory usage:

    * The size of each state is 237 bits (rounded up to 30 bytes).
    * The size of the hash table is 16384 slots.

  Progress Report:

     thread 5: 10000 states explored in 0s, with 2036 rules fired and 465 states in the queue.
     thread 6: 20000 states explored in 0s, with 4666 rules fired and 695 states in the queue.
     thread 2: 30000 states explored in 0s, with 7593 rules fired and 795 states in the queue.
     thread 7: 40000 states explored in 0s, with 10694 rules fired and 976 states in the queue.
     thread 2: 50000 states explored in 0s, with 13427 rules fired and 581 states in the queue.
  The following is the error trace for the error:

    invariant 1 failed

  Startstate 1 fired.
  total:0
  tls[0].pc:LOOP_START
  tls[0].i:0
  tls[0].tmp:Undefined
  tls[1].pc:LOOP_START
  tls[1].i:0
  tls[1].tmp:Undefined
  tls[2].pc:LOOP_START
  tls[2].i:0
  tls[2].tmp:Undefined
  ----------

  Rule "tmp = total", t: 0 fired.
  tls[0].pc:READ_TOTAL
  tls[0].tmp:0
  ----------

  Rule "tmp = total", t: 2 fired.
  tls[2].pc:READ_TOTAL
  tls[2].tmp:0
  ----------

  Rule "total = tmp + 1", t: 0 fired.
  total:1
  tls[0].pc:LOOP_START
  tls[0].i:1
  ----------

  Rule "tmp = total", t: 0 fired.
  tls[0].pc:READ_TOTAL
  tls[0].tmp:1
  ----------

  Rule "tmp = total", t: 1 fired.
  tls[1].pc:READ_TOTAL
  tls[1].tmp:1
  ----------

  Rule "total = tmp + 1", t: 0 fired.
  total:2
  tls[0].pc:LOOP_START
  tls[0].i:2
  ----------

  Rule "tmp = total", t: 0 fired.
  tls[0].pc:READ_TOTAL
  tls[0].tmp:2
  ----------

  Rule "total = tmp + 1", t: 0 fired.
  total:3
  tls[0].pc:LOOP_START
  tls[0].i:3
  ----------

  Rule "total = tmp + 1", t: 1 fired.
  total:2
  tls[1].pc:LOOP_START
  tls[1].i:1
  ----------

  Rule "tmp = total", t: 1 fired.
  tls[1].pc:READ_TOTAL
  tls[1].tmp:2
  ----------

  Rule "total = tmp + 1", t: 1 fired.
  total:3
  tls[1].pc:LOOP_START
  tls[1].i:2
  ----------

  Rule "tmp = total", t: 1 fired.
  tls[1].pc:READ_TOTAL
  tls[1].tmp:3
  ----------

  Rule "total = tmp + 1", t: 1 fired.
  total:4
  tls[1].pc:LOOP_START
  tls[1].i:3
  ----------

  Rule "total = tmp + 1", t: 2 fired.
  total:1
  tls[2].pc:LOOP_START
  tls[2].i:1
  ----------

  Rule "tmp = total", t: 2 fired.
  tls[2].pc:READ_TOTAL
  tls[2].tmp:1
  ----------

  Rule "total = tmp + 1", t: 2 fired.
  total:2
  tls[2].pc:LOOP_START
  tls[2].i:2
  ----------

  Rule "tmp = total", t: 2 fired.
  tls[2].pc:READ_TOTAL
  tls[2].tmp:2
  ----------

  Rule "total = tmp + 1", t: 2 fired.
  total:3
  tls[2].pc:LOOP_START
  tls[2].i:3
  ----------

  Rule "exit loop", t: 0 fired.
  tls[0].pc:DONE
  ----------

  Rule "exit loop", t: 1 fired.
  tls[1].pc:DONE
  ----------

  Rule "exit loop", t: 2 fired.
  tls[2].pc:DONE
  ----------

  End of the error trace.


  ==========================================================================

  Status:

    1 error(s) found.

  State Space Explored:

    52816 states, 117240 rules fired in 0s.

The trace may look long and daunting, but that is only because each step is
small and Rumur is giving us precise details on how to reproduce the bug.
Converting this back into implementation steps, it can more concisely be
expressed as:

.. code-block::

  thread 0        thread 1        thread 2        total
  --------------------------------------------------------
  tmp = total                                     0
                                  tmp = total     0
  total = tmp + 1                                 1
  tmp = total                                     1
                  tmp = total                     1
  total = tmp + 1                                 2
  tmp = total                                     2
  total = tmp + 1                                 3
                  total = tmp + 1                 2
                  tmp = total                     2
                  total = tmp + 1                 3
                  tmp = total                     3
                  total = tmp + 1                 4
                                  total = tmp + 1 1
                                  tmp = total     1
                                  total = tmp + 1 2
                                  tmp = total     2
                                  total = tmp + 1 3
  exit loop                                       3
                  exit loop                       3
                                  exit loop

Translating a counterexample trace into an intuition of what occurred is a skill
in itself and, like abstraction, comes with experience.

So our program can run to completion with a final ``total`` lower than what was
expected. How to fix the program to behave correctly is left as an exercise to
the reader. Creating a concurrent system that avoids all possible bugs is not
something Rumur solves for you: it can find your bugs or prove their absence,
but actually designing the system is your job.

Further Optimisations
---------------------
Something you may have noticed is that all 3 threads were not necessary to
reproduce this bug. We could have omitted thread 1 (while also reducing the
expected ``total`` to 6) and Rumur would have still found the bug. It does not
make a significant difference in this case, but in larger systems the difference
between 2 and 3 threads can mean the difference between a checkable system and
exhausting resources. Furthermore a counterexample trace involving fewer threads
is generally easier to understand.

If your system naturally has threads like this example, a good practice is to
run your model with 2 until you cannot find any more bugs. Then increase to 3
and run like that until you cannot find any more bugs. Continue this until you
are resource constrained or gain enough confidence in your design.

To reduce the amount of memory required for state space exploration, *symmetry
reduction* can be used. This is an optimisation that teaches Rumur that parts of
your system are semantically equivalent (e.g. thread 0 and thread 1 are
interchangeable). For further information about this, look into the
``scalarset`` Murphi keyword.

Another advanced technique is to eagerly undefine variables. In our example, the
value of ``tls[t].tmp`` is irrelevant unless the thread is at the ``READ_TOTAL``
label. But it will contribute unnecessary interleavings in state space
exploration, increasing memory requirements. These can be minimised by
explicitly discarding a value you know will not be read again:

.. code-block:: diff

   rule "total = tmp + 1"
       tls[t].pc = READ_TOTAL
     ==>
   begin
     total := tls[t].tmp + 1;
     tls[t].i := tls[t].i + 1;
  +  undefine tls[t].tmp;
     tls[t].pc := LOOP_START;
   end

When combined with symmetry reduction, this can lead to significant memory
savings. This can also help catch bugs in your model itself. Reading an
undefined variable triggers an error, so undefining a variable allows Rumur to
detect later unintended reads of it.

Final Model
-----------
The full model we constructed above is included here for reference:

.. code-block:: murphi

  const INT_MIN: -2147483648;
  const INT_MAX: 2147483647;
  type int: INT_MIN..INT_MAX;

  var total: int;

  const THREADS: 3;
  type thread_id: 0..THREADS-1;

  type label_t: enum { LOOP_START, READ_TOTAL, DONE }

  type tls_t: record
         pc: label_t;
         i: int;
         tmp: int;
       end;
  var tls: array[thread_id] of tls_t;

  ruleset t: thread_id do
    rule "tmp = total"
        tls[t].pc = LOOP_START
      & tls[t].i < 3
      ==>
    begin
      tls[t].tmp := total;
      tls[t].pc := READ_TOTAL;
    end

    rule "total = tmp + 1"
        tls[t].pc = READ_TOTAL
      ==>
    begin
      total := tls[t].tmp + 1;
      tls[t].i := tls[t].i + 1;
      tls[t].pc := LOOP_START;
    end

    rule "exit loop"
        tls[t].pc = LOOP_START
      & tls[t].i >= 3
      ==>
    begin
      tls[t].pc := DONE;
    end;
  end

  startstate begin
    total := 0;
    for t: thread_id do
      tls[t].pc := LOOP_START;
      tls[t].i := 0;
    end;
  end

  -- if we are done, the counter is at its expected value
  invariant
    forall t: thread_id do tls[t].pc = DONE end
    ->
    total = 9;
