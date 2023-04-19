Introduction to Murphi
======================
The input syntax accepted by Rumur is a variant of a language called “Murphi”
that was originally designed for use in a different model checker. The following
provides an introduction to the language by way of a simple example that we
gradually build upon.

Overview
--------
Murphi models consist of three classes of definitions:

1. Definitions of constants, types and state variables;
2. Functions and procedures; and
3. State transition rules and invariants.

Rumur’s predecessors required these to appear strictly in order. That is, all
constants, types and state variables had to be defined before any functions or
procedures, and all functions and procedures had to be defined before any rules
or invariants. Rumur does not have this restriction, but it is a good habit to
observe this if you want your model to be compatible with other Murphi tools.

The Dining Philosophers
-----------------------
Let’s think about a classic concurrency problem, the dining philosophers. Five
philosophers go out for a spaghetti dinner. They sit around a circular table
with five plates of spaghetti and five forks, placed between the plates. Each
philosopher needs two forks to eat the plate of spaghetti in front of them.

This is a system that can be thought of as a state machine. At each point in
time, a philosopher can either be thinking, hungry and trying to eat, or eating.
The moves they can each make are to pick up a fork or put down a fork. We want
to ensure that all philosophers eventually get to eat. Sounds pretty simple,
right?

Building a Model
----------------
We can use Rumur to prove all the philosophers will eventually get to eat. To do
this, we construct a Murphi model describing the dining philosophers.

Our philosophy dinner has five participants, but maybe in future we will want to
think about more than five philosophers. So let’s plan ahead and use a Murphi
constant that we can easily update later.

.. code-block:: murphi

  const N_PHILOSOPHERS: 5

We always have the same number of forks as philosophers, so let’s define that
too.

.. code-block:: murphi

  const N_FORKS N_PHILOSOPHERS

We can identify each philosopher and each fork by an identifier from 0 to 4.

.. code-block:: murphi

  type philosopher_id: 0 .. N_PHILOSOPHERS - 1
  type fork_id: 0 .. N_FORKS - 1

The philosophers can be thinking, hungry or eating. We can represent this in
Murphi with an enumerated type.

.. code-block:: murphi

  type action: enum { THINKING, HUNGRY, EATING }

We can now represent a philosopher as a record (like a C struct) capturing their
mood and which forks they have.

.. code-block:: murphi

  type philosopher: record
         mood: action
         has_left_fork: boolean
         has_right_fork: boolean
       end

We’re now ready to define the state of the system. This will be the philosophers
themselves and an array indicating which forks are held.

.. code-block:: murphi

  var philosophers: array[philosopher_id] of philosopher
  var forks: array[fork_id] of boolean

This completes the first section of our dining philosophers model. We have seen
constants, all the basic types, and how to define state variables. The keywords
``const``, ``type``, and ``var`` are not actually attached to individual
declarations but rather to sections. So we could write a condensed version of
what we have so far as follows.

.. code-block:: murphi

  const
    N_PHILOSOPHERS: 5
    N_FORKS: N_PHILOSOPHERS

  type
    philosopher_id: 0 .. N_PHILOSOPHERS - 1
    fork_id: 0 .. N_FORKS - 1
    action: enum { THINKING, HUNGRY, EATING }
    philosopher: record
      mood: action
      has_left_fork: boolean
      has_right_fork: boolean
    end

  var
    philosophers: array[philosopher_id] of philosopher
    forks: array[fork_id] of boolean

OK, time for some helper functions. We already know we will need to refer to the
forks on either side of a philosopher. So let’s define functions for these to
avoid having to write awkward modular arithmetic repeatedly.

.. code-block:: murphi

  function left_fork(id: philosopher_id): fork_id; begin
    return id;
  end

  function right_fork(id: philosopher_id): fork_id; begin
    return (id + 1) % N_PHILOSOPHERS;
  end

Note that the way parameters and return types occur might be back to front to
what you expect from, for example, C. Function parameters are given as
``name: type`` and the return type appears after the function’s closing bracket.

We can now start defining the state transition rules for our system. These are
(optionally guarded) blocks that describe updates to the state. All the
statements within a rule execute atomically. That is, the entire block as a
whole represents a transition from one system state to another. The first one of
these rules is the start state that describes how to initialise our system.

.. code-block:: murphi

  startstate begin

    -- all philosophers start dinner hungry and empty handed
    for i: philosopher_id do
      philosophers[i].mood := HUNGRY;
      philosophers[i].has_left_fork := false;
      philosophers[i].has_right_fork := false;
    end;

    -- so all forks are unheld
    for i: fork_id do
      forks[i] := false;
    end;

  end

A Murphi model can have multiple start states. However we will only use one in
this model.

What do the philosophers do when they are hungry? They try to grab forks to eat.
To write a transition rule for this we do not want to talk about any particular
philosopher but rather *any* of the five philosophers. We can do this with a
rule set.

.. code-block:: murphi

  ruleset i: philosopher_id do

    rule "take left fork"
        philosophers[i].mood = HUNGRY   -- wants to eat
      & !philosophers[i].has_left_fork  -- doesn’t have the left fork
      & !forks[left_fork(i)]            -- left fork is available
      ==>
    begin
      forks[left_fork(i)] := true;
      philosophers[i].has_left_fork := true;
    end

    rule "take right fork"
        philosophers[i].mood = HUNGRY
      & !philosophers[i].has_right_fork
      & !forks[right_fork(i)]
      ==>
    begin
      forks[right_fork(i)] := true;
      philosophers[i].has_right_fork := true;
    end

  end

If a philosopher has both their forks and they are hungry, they can start
eating.

.. code-block:: murphi

  ruleset i: philosopher_id do

    rule "eat"
        philosophers[i].mood = HUNGRY   -- wants to eat
      & philosophers[i].has_left_fork
      & philosophers[i].has_right_fork  -- has both forks
      ==>
    begin
      philosophers[i].mood := EATING;
    end

  end

Once they have been eating, a philosopher may get full and decide to take a
break and think for a while. Note that at our dinner philosophers cannot start
thinking when they are hungry because they are too distracted by their stomach
rumbling.

.. code-block:: murphi

  ruleset i: philosopher_id do

    rule "think"
      philosophers[i].mood = EATING ==>
    begin
      philosophers[i].mood := THINKING;
    end

  end

A thinking philosopher may always be struck by hunger again.

.. code-block:: murphi

  ruleset i: philosopher_id do

    rule "get hungry"
      philosophers[i].mood = THINKING ==>
    begin
      philosophers[i].mood := HUNGRY;
    end

  end

Finally, a philosopher who is thinking but also holding forks places them back
on the table as they ponder the mysteries of the universe.

.. code-block:: murphi

  ruleset i: philosopher_id do

    rule "drop left fork"
        philosophers[i].mood = THINKING
      & philosophers[i].has_left_fork
      ==>
    begin
      -- sanity check that the fork we are releasing was held
      assert forks[left_fork(i)];
      forks[left_fork(i)] := false;
      philosophers[i].has_left_fork := false;
    end

    rule "drop right fork"
        philosophers[i].mood = THINKING
      & philosophers[i].has_right_fork
      ==>
    begin
      assert forks[right_fork(i)];
      forks[right_fork(i)] := false;
      philosophers[i].has_right_fork := false;
    end

  end

These are all the transition rules we need to describe the dining philosophers.
Let’s add an invariant; something that should always be true. Each fork can only
be held by a single philosopher, so let’s claim that a philosopher holding their
right fork is always next to a philosopher missing their left fork.

.. code-block:: murphi

  invariant "no fork sharing"

    -- for any philosopher...
    forall i: philosopher_id do

      -- ...either they do not have their right fork...
      !philosophers[i].has_right_fork

      -- ...or their neighbour does not have their left fork
      | !philosophers[(i + 1) % N_PHILOSOPHERS].has_left_fork

    end

And that’s it! We have built a model of the dining philosophers. Let’s put
is altogether and merge the rule sets into a single block.

.. code-block:: murphi

  const
    N_PHILOSOPHERS: 5
    N_FORKS: N_PHILOSOPHERS

  type
    philosopher_id: 0 .. N_PHILOSOPHERS - 1
    fork_id: 0 .. N_FORKS - 1
    action: enum { THINKING, HUNGRY, EATING }
    philosopher: record
      mood: action
      has_left_fork: boolean
      has_right_fork: boolean
    end

  var
    philosophers: array[philosopher_id] of philosopher
    forks: array[fork_id] of boolean

  function left_fork(id: philosopher_id): fork_id; begin
    return id;
  end

  function right_fork(id: philosopher_id): fork_id; begin
    return (id + 1) % N_PHILOSOPHERS;
  end

  startstate begin

    -- all philosophers start dinner hungry and empty handed
    for i: philosopher_id do
      philosophers[i].mood := HUNGRY;
      philosophers[i].has_left_fork := false;
      philosophers[i].has_right_fork := false;
    end;

    -- so all forks are unheld
    for i: fork_id do
      forks[i] := false;
    end;

  end

  ruleset i: philosopher_id do

    rule "take left fork"
        philosophers[i].mood = HUNGRY   -- wants to eat
      & !philosophers[i].has_left_fork  -- doesn’t have the left fork
      & !forks[left_fork(i)]            -- left fork is available
      ==>
    begin
      forks[left_fork(i)] := true;
      philosophers[i].has_left_fork := true;
    end

    rule "take right fork"
        philosophers[i].mood = HUNGRY
      & !philosophers[i].has_right_fork
      & !forks[right_fork(i)]
      ==>
    begin
      forks[right_fork(i)] := true;
      philosophers[i].has_right_fork := true;
    end

    rule "eat"
        philosophers[i].mood = HUNGRY   -- wants to eat
      & philosophers[i].has_left_fork
      & philosophers[i].has_right_fork  -- has both forks
      ==>
    begin
      philosophers[i].mood := EATING;
    end

    rule "think"
      philosophers[i].mood = EATING ==>
    begin
      philosophers[i].mood := THINKING;
    end

    rule "get hungry"
      philosophers[i].mood = THINKING ==>
    begin
      philosophers[i].mood := HUNGRY;
    end

    rule "drop left fork"
        philosophers[i].mood = THINKING
      & philosophers[i].has_left_fork
      ==>
    begin
      assert forks[left_fork(i)];
      forks[left_fork(i)] := false;
      philosophers[i].has_left_fork := false;
    end

    rule "drop right fork"
        philosophers[i].mood = THINKING
      & philosophers[i].has_right_fork
      ==>
    begin
      assert forks[right_fork(i)];
      forks[right_fork(i)] := false;
      philosophers[i].has_right_fork := false;
    end

  end

  invariant "no fork sharing"

    -- for any philosopher...
    forall i: philosopher_id do

      -- ...either they do not have their right fork...
      !philosophers[i].has_right_fork

      -- ...or their neighbour does not have their left fork
      | !philosophers[(i + 1) % N_PHILOSOPHERS].has_left_fork

    end

Now to see what Rumur can tell us about this system...

Verifying the Model
-------------------
We can take the model we have just written and ask Rumur to produce a program to
verify it. That is, Rumur will generate a separate C program that captures the
meaning of our model and will check its properties for us. Save the model as
philosophers.m and then run the following.

.. code-block:: sh

  rumur --output philosophers.c philosophers.m

We now have a file, philosophers.c, that is our verifier. We can compile this
with a C compiler.

.. code-block:: sh

  # if you are using an x86-64 machine, also add -mcx16
  cc -std=c11 -O3 -o philosophers philosophers.c -lpthread

Now we can run the verifier to try and prove our model correct.

.. code-block:: sh

  ./philosophers
  Memory usage:

    * The size of each state is 40 bits (rounded up to 5 bytes).
    * The size of the hash table is 32768 slots.

  Progress Report:

  The following is the error trace for the error:

    deadlock

  Startstate 1 fired.
  philosophers[0].mood:HUNGRY
  philosophers[0].has_left_fork:false
  philosophers[0].has_right_fork:false
  philosophers[1].mood:HUNGRY
  philosophers[1].has_left_fork:false
  philosophers[1].has_right_fork:false
  philosophers[2].mood:HUNGRY
  philosophers[2].has_left_fork:false
  philosophers[2].has_right_fork:false
  philosophers[3].mood:HUNGRY
  philosophers[3].has_left_fork:false
  philosophers[3].has_right_fork:false
  philosophers[4].mood:HUNGRY
  philosophers[4].has_left_fork:false
  philosophers[4].has_right_fork:false
  forks[0]:false
  forks[1]:false
  forks[2]:false
  forks[3]:false
  forks[4]:false
  ----------

  Rule "take left fork", i: 0 fired.
  philosophers[0].has_left_fork:true
  forks[0]:true
  ----------

  Rule "take left fork", i: 1 fired.
  philosophers[1].has_left_fork:true
  forks[1]:true
  ----------

  Rule "take left fork", i: 2 fired.
  philosophers[2].has_left_fork:true
  forks[2]:true
  ----------

  Rule "take left fork", i: 3 fired.
  philosophers[3].has_left_fork:true
  forks[3]:true
  ----------

  Rule "take left fork", i: 4 fired.
  philosophers[4].has_left_fork:true
  forks[4]:true
  ----------

  End of the error trace.


  ==========================================================================

  Status:

    1 error(s) found.

  State Space Explored:

    597 states, 1734 rules fired in 0s.

Hm, it pretty clearly failed but why? And what does all of this output mean?
We can see the failure cause at towards the beginning of the output, “deadlock.”
The verifier found a sequence of transitions that would result in a state where
none of the philosophers could make a move.

The blocks of output following the error itself give a counterexample trace.
This shows the exact path of rule transitions needed to reproduce the deadlocked
state. By following this, you can see we have found a well known problem with
this classic example, where each philosopher takes the fork to their left. After
this, all philosophers are wanting the fork to their right but the fork to the
right of each philosopher is already held by their neighbour.

There are a couple of noteworthy points here. Rumur found a problem in our model
quickly and without us having to guide it. It also found a problem that was not
a violation of our invariant, but a problem it knew to check for anyway. By
default, Rumur considers a deadlock of your model to be an error condition.

Fixing the Model
----------------
So the design of our system is incorrect, and model checking helped us find the
problem with it. How do we go about correcting it?

Let’s take one of the known solutions to this problem and order the resources
(forks) that are being acquired. Instead of letting a philosopher take any fork,
we say they can only take the free fork with the lowest identifier of the two
they need.

.. code-block:: diff

  ruleset i: philosopher_id do

    rule "take left fork"
        philosophers[i].mood = HUNGRY   -- wants to eat
      & !philosophers[i].has_left_fork  -- doesn’t have the left fork
  +    -- either already has the right fork or the left is lower numbered
  +    & (philosophers[i].has_right_fork | left_fork(i) < right_fork(i))
      & !forks[left_fork(i)]            -- left fork is available
      ==>
    begin
      forks[left_fork(i)] := true;
      philosophers[i].has_left_fork := true;
    end

    rule "take right fork"
        philosophers[i].mood = HUNGRY
      & !philosophers[i].has_right_fork
  +    & (philosophers[i].has_left_fork | left_fork(i) > right_fork(i))
      & !forks[right_fork(i)]
      ==>
    begin
      forks[right_fork(i)] := true;
      philosophers[i].has_right_fork := true;
    end

  end

We also need to apply this ordering on fork release, and only allow philosophers
to release forks in descending order.

.. code-block:: diff

  ruleset i: philosopher_id do

    rule "drop left fork"
        philosophers[i].mood = THINKING
      & philosophers[i].has_left_fork
  +    -- either right is not held or the left is higher numbered
  +    & (!philosophers[i].has_right_fork | left_fork(i) > right_fork(i))
      ==>
    begin
      assert forks[left_fork(i)];
      forks[left_fork(i)] := false;
      philosophers[i].has_left_fork := false;
    end

    rule "drop right fork"
        philosophers[i].mood = THINKING
      & philosophers[i].has_right_fork
  +    & (!philosophers[i].has_left_fork | left_fork(i) < right_fork(i))
      ==>
    begin
      assert forks[right_fork(i)];
      forks[right_fork(i)] := false;
      philosophers[i].has_right_fork := false;
    end

  end

Is this enough? Let’s ask Rumur.

.. code-block:: sh

  rumur --output philosophers.c philosophers.m
  cc -std=c11 -O3 -o philosophers philosophers.c -lpthread
  ./philosophers
  Memory usage:

    * The size of each state is 40 bits (rounded up to 5 bytes).
    * The size of the hash table is 32768 slots.

  Progress Report:


  ==========================================================================

  Status:

    No error found.

  State Space Explored:

    3216 states, 16160 rules fired in 0s.

Hooray! Our model is deadlock free and our invariant is proven. From here, maybe
we would like to increase the number of philosophers or make the dining
arrangement more complicated. We could also introduce further invariants to
check the model more thoroughly. But hopefully you have already seen enough to
understand the value of model checking and have some ideas about how to write
models of your own systems.

Bonus - Liveness
----------------
At this point, we have proven that the philosophers do not get stuck and do not
share forks, but we have not shown that each philosopher does get the chance to
eat. In fact, we would like to go even further than that and prove that they are
always able to eat.[1]_ To do this, we write a liveness property.

.. [1] This might be phrased as that they are able to eat infinitely often, but
  this sounds a little peculiar.

.. code-block:: murphi

  ruleset i: philosopher_id do

    liveness "can eventually eat"
      philosophers[i].mood = EATING

  end

We need to rebuild and compile our model, then run it to see the result.

.. code-block:: sh

  rumur --output philosophers.c philosophers.m
  cc -std=c11 -O3 -o philosophers philosophers.c -lpthread
  ./philosophers
  Memory usage:

    * The size of each state is 40 bits (rounded up to 5 bytes).
    * The size of the hash table is 32768 slots.

  Progress Report:


  ==========================================================================

  Status:

    No error found.

  State Space Explored:

    3216 states, 16160 rules fired in 0s.

And with that, we have proven that our philosophers will not starve but can all
eat as much as they please.

For reference, the full model we constructed is given below.

.. code-block:: murphi

  const
    N_PHILOSOPHERS: 5
    N_FORKS: N_PHILOSOPHERS

  type
    philosopher_id: 0 .. N_PHILOSOPHERS - 1
    fork_id: 0 .. N_FORKS - 1
    action: enum { THINKING, HUNGRY, EATING }
    philosopher: record
      mood: action
      has_left_fork: boolean
      has_right_fork: boolean
    end

  var
    philosophers: array[philosopher_id] of philosopher
    forks: array[fork_id] of boolean

  function left_fork(id: philosopher_id): fork_id; begin
    return id;
  end

  function right_fork(id: philosopher_id): fork_id; begin
    return (id + 1) % N_PHILOSOPHERS;
  end

  startstate begin

    -- all philosophers start dinner hungry and empty handed
    for i: philosopher_id do
      philosophers[i].mood := HUNGRY;
      philosophers[i].has_left_fork := false;
      philosophers[i].has_right_fork := false;
    end;

    -- so all forks are unheld
    for i: fork_id do
      forks[i] := false;
    end;

  end

  ruleset i: philosopher_id do

    rule "take left fork"
        philosophers[i].mood = HUNGRY   -- wants to eat
      & !philosophers[i].has_left_fork  -- doesn’t have the left fork
      -- either already has the right fork or the left is lower numbered
      & (philosophers[i].has_right_fork | left_fork(i) < right_fork(i))
      & !forks[left_fork(i)]            -- left fork is available
      ==>
    begin
      forks[left_fork(i)] := true;
      philosophers[i].has_left_fork := true;
    end

    rule "take right fork"
        philosophers[i].mood = HUNGRY
      & !philosophers[i].has_right_fork
      & (philosophers[i].has_left_fork | left_fork(i) > right_fork(i))
      & !forks[right_fork(i)]
      ==>
    begin
      forks[right_fork(i)] := true;
      philosophers[i].has_right_fork := true;
    end

    rule "eat"
        philosophers[i].mood = HUNGRY   -- wants to eat
      & philosophers[i].has_left_fork
      & philosophers[i].has_right_fork  -- has both forks
      ==>
    begin
      philosophers[i].mood := EATING;
    end

    rule "think"
      philosophers[i].mood = EATING ==>
    begin
      philosophers[i].mood := THINKING;
    end

    rule "get hungry"
      philosophers[i].mood = THINKING ==>
    begin
      philosophers[i].mood := HUNGRY;
    end

    rule "drop left fork"
        philosophers[i].mood = THINKING
      & philosophers[i].has_left_fork
      -- either right is not held or the left is higher numbered
      & (!philosophers[i].has_right_fork | left_fork(i) > right_fork(i))
      ==>
    begin
      assert forks[left_fork(i)];
      forks[left_fork(i)] := false;
      philosophers[i].has_left_fork := false;
    end

    rule "drop right fork"
        philosophers[i].mood = THINKING
      & philosophers[i].has_right_fork
      & (!philosophers[i].has_left_fork | left_fork(i) < right_fork(i))
      ==>
    begin
      assert forks[right_fork(i)];
      forks[right_fork(i)] := false;
      philosophers[i].has_right_fork := false;
    end

  end

  invariant "no fork sharing"

    -- for any philosopher...
    forall i: philosopher_id do

      -- ...either they do not have their right fork...
      !philosophers[i].has_right_fork

      -- ...or their neighbour does not have their left fork
      | !philosophers[(i + 1) % N_PHILOSOPHERS].has_left_fork

    end

  ruleset i: philosopher_id do

    liveness "can eventually eat"
      philosophers[i].mood = EATING

  end

Suggested to read next: `Murphi Idioms`_

.. _`Murphi Idioms`: murphi-idioms.rst
