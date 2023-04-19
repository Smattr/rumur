An Introduction to Rumur
========================
Rumur is a tool for formal verification of complex systems. It falls into a
family known as “model checkers.” This document gives a brief introduction to
formal verification and model checking, aimed at helping Rumur users understand
at a high level how the tool works.

Formal Verification
-------------------
When building a complex system, you would like to know that it is correct.
“Correct” is subjective and something specific to a particular system, but a
person building a system generally knows what they expect that system to do (and
what they expect it *not* to do).

The traditional way of gaining confidence in a piece of software is to write
tests. But as Dijkstra famously said, “testing can be used to show the presence
of bugs, but never to show their absence.” Formal verification approaches this
problem from the other direction. Instead of writing tests to give your system
specific inputs and relying on the cleverness of the test author to exercise all
the system’s edge cases, formal verification covers *all possible inputs*.

When first encountering this family of techniques they may seem like magic, and
some of the more advanced formal verification approaches are indeed complex to
understand. However, Rumur is a particular type of formal verification tool
called a “model checker” whose operation is actually quite simple, as we shall
see.

Model Checking
--------------
Suppose you built a system that took the result of the roll of a six-sided die
as input. What tests would you write? One test for each of the inputs (1, 2, 3,
4, 5, 6) would cover all possible scenarios. This is what model checking does on
a larger scale. It simply tries every possible path through your system.

Of course, for most systems this is not possible. For example, suppose the die
you were rolling had 2⁶⁴ sides. You cannot try this many values in a reasonable
amount of time. Here the key is to *abstract* your system. Does this larger
system behave sufficiently differently to the smaller one that takes a six-sided
die? If not, maybe it is reasonable to verify the correctness of the smaller
system as a stand-in for the larger one.

While the type of abstraction just described needs to be done by hand, there are
certain types of abstraction that can be done automatically. Some model checking
tools can identify *symmetries* in your system; situations where two paths
through a system are equivalent and only one need be explored. With real world
systems, this can be a powerful optimisation that transforms infeasible
verification problems into something achievable.

Suggested to read next: `Introduction to Murphi`_

.. _`Introduction to Murphi`: intro-to-murphi.rst
