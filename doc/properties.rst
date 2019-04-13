Properties
==========
When creating a model of a system, an important part to include is its
definition of correctness. Rumur gives you four different types of global
properties you can write: invariants, covers, assumptions, and liveness. Each of
these except liveness can also be used locally (within a function, procedure, or
rule). However, the local equivalent of an invariant is known as an assertion.

Invariants
----------
An invariant is something you claim is always true following the execution of a
rule. That is, a global correctness criterion for your system. You state an
invariant with the ``invariant`` keyword. For example, you might model a counter
that always has an even value:

.. code-block:: murphi

  var
    counter: 0 .. 10;

  startstate begin
    counter := 0;
  end;

  rule "increment"
    counter < 10 ==>
  begin
    counter := counter + 2;
  end;

  rule "decrement"
    counter > 0 ==>
  begin
    counter := counter - 2;
  end;

  invariant "counter is always even"
    counter % 2 = 0;

When you generate a verifier for this model and then run it, the verifier will
check each state satisfies the condition ``counter % 2 = 0``. You can observe
this by changing the property to something that will fail, like
``counter % 4 = 0``, and re-checking the model.

Invariants can appear within rulesets, so you can write a parameterised
invariant:

.. code-block:: murphi

  ruleset x: 0 .. 10 do
    invariant "x is small" x < 100;
  end;

This example is equivalent to:

.. code-block:: murphi

  invariant "x is small"
    forall x: 0 .. 10 do x < 100 end;

but sometimes it is more natural to use the ruleset style of writing. The text
given to describe an invariant is optional, but it is good practice to include
a description of the property to make invariant failure messages clearer.

The keyword ``invariant`` and the keyword ``assert`` (see below) are treated as
synonyms by Rumur, but it is good practice to use ``invariant`` for top level
properties to better indicate your intentions to readers.

Assertions
----------
Assertions are similar to invariants, but are only expected to hold locally at
the point at which they are executed. You may be familiar with them from
conventional programming languages. Re-using the counter example from above, you
could add an assertion to the "increment" rule:

.. code-block:: murphi

  rule "increment"
    counter < 10 ==>
  begin
    counter := counter + 2;
    assert "counter is non-zero" counter != 0;
  end;

When the "increment" rule is run, this condition will be checked after
incrementing ``counter``. Like invariants, the message is optional but
encouraged. As mentioned above, ``invariant`` and ``assert`` are synonyms but it
is recommended to use ``assert`` for local properties to aid readability.

Covers
------
Similar to invariants and assertions, covers express a property you expect to
hold but not necessarily every time it is checked. A cover is typically used to
describe a reachability condition of your system. The generated checker counts
how many times a cover property was found to be true and considers it an error
if there is a cover that was never found to be true.

Using our previous counter model, you could write a global cover property to
ensure the counter is ``8`` at some point during execution:

.. code-block:: murphi

  cover "counter seen as 8" counter = 8;

Covers can be either global (like the example above) or local. You can use a
local cover to check path coverage within a rule. For example,

.. code-block:: murphi

  rule "increment"
    counter < 10 ==>
  begin
    if counter = 2 then
      cover "if branch covered" true;
      counter := counter + 4;
    else
      cover "else branch covered" true;
      counter := counter + 2;
    end;
  end;

If you want to claim something is always true but also count the number of times
you check the property, you can combine an assertion and a cover:

.. code-block:: murphi

  rule "increment"
    counter < 10 ==>
  begin
    counter := counter + 2;
    assert "counter is even" counter % 2 = 0;
    cover "counter is even" counter % 2 = 0;
  end;

Assumptions
-----------
The properties discussed thus far allow you to write conditions you want to
check hold in your system. Assumptions are a way to describe conditions you do
not want to check but wish to assume of the environment your system operates in.
For example, the "increment" rule could be written without a guard, instead
using an assumption to avoid thinking about overflow:

.. code-block:: murphi

  rule "increment" begin
    assume "counter within bounds" counter < 10;
    counter := counter + 2;
  end;

This causes any transition that encounters a value of ``counter`` not less than
``10`` to be considered invalid. Like the other properties, assumptions can be
either global or local, so we could write a global assumption that prevents the
counter ever reaching ``10``:

.. code-block:: murphi

  assume "clamp counter" counter < 8;

For the reader who is curious about how this is implemented, the generated
verifier discards any state that violates an assumption. That is, an assumption
failure (either local within a rule or global after a rule transition) causes
the invariant and cover checks to be skipped and the resulting (invalid) state
to be ignored.

Liveness
--------
Liveness properties express something that should be reachable from every state.
That is, a property that may not be true in a given state but will be true in at
least one of the successors of every state. Using the counter example, we might
write,

.. code-block:: murphi

  liveness "counter can always become 4" counter = 4;

to capture that we should always be able to reach the state where ``counter`` is
``4``. Unlike the other property types, liveness can only be used globally and
not locally as a statement.

Sometimes you might want to write a conditional liveness property. For example,
that a variable ``x`` can always become ``1`` but only once you are out of some
initial setup phase and into the steady state of your system. You can do this by
also referencing a variable that captures the phase of your system in the
liveness property. For example,

.. code-block:: murphi

  type
    phase_t: enum { SETUP, RUN };

  var
    phase: phase_t;
    halt: boolean;
    x: 0 .. 10;

  startstate begin
    phase := SETUP;
    halt := false;
  end;

  ruleset has_err: boolean do
    rule "init" !halt & phase = SETUP ==> begin
      if has_err then
        halt := true;
      else
        x := 0;
        phase := RUN;
      end;
    end;
  end;

  rule phase = RUN & x < 10 ==> begin
    x := x + 1;
  end;

  rule phase = RUN & x > 0 ==> begin
    x := x - 1;
  end;

  liveness "x can be 1" phase = SETUP | x = 1;

For large models, note that Rumur's algorithm for checking liveness properties
is not as efficient as other types of properties. You may find that checking a
liveness properties on a large state space requires a long time.

Relationship to Linear Temporal Logic
-------------------------------------
Readers familiar with Linear Temporal Logic (LTL) might notice a similarity in
the properties supported by Rumur and the expressibility of LTL. Rumur's
available properties are more constrained than LTL. To be pragmatic, they are
limited to some that can be checked efficiently within an explicit state model
checking algorithm.

Invariants roughly correspond to LTL's "always" operator, ``G`` or ``□``.
Liveness roughly corresponds to "always eventually", ``G F`` or ``□ ◊``.
The other types of properties do not have direct LTL equivalents.
