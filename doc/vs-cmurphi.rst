Rumur vs CMurphi
================
Because Rumur is attempting to appeal to CMurphi users, it is helpful to compare
the feature set of the two. In this document we give a brief run down of the
relevant differences.

Performance Characteristics
---------------------------
CMurphi produces a single-threaded verifier. If you have a multicore system, the
verifier will only take advantage of a single core. Rumur produces a
multi-threaded verifier by default, which will use as many cores as you have
available. You can tune how many threads are used with the ``--threads`` command
line option when generating the verifier.

CMurphi seems intended to run on Linux and older Unix-style platforms. Rumur
should run on any POSIX operating system. However, the verifier’s code does make
use of C extensions that may only be supported by GCC and Clang.

Syntax Differences
------------------
CMurphi considers the operator ``==`` an error, while Rumur treats it as an
alias of ``=`` that is usable in comparisons. This is mainly to ease the life of
C/C++ programmers.

Rumur supports various single character operator alternatives, e.g. ``≤``. These
are nicer for presentation of a model in a mathematical context. These more
concise operators are generally recommended for use in models that are expected
to be read more often than they are edited.

The keywords ``assert`` and ``invariant`` are distinct in CMurphi, whereas they
are considered synonyms by Rumur. If one of these appears within a rule or
function it is considered an assertion and if it appears at the top level it is
considered an invariant. Rumur also accepts the optional string message of an
assertion or invariant on either side of the asserted expression.

Type System
-----------
CMurphi supports real arithmetic using the ``real`` data type. Rumur does not
support this type and there are no plans to implement this or any floating point
support. Similarly, Rumur does not support the ``union`` type and there are no
plans to add it.

Assumptions
-----------
In addition to assertions and invariants that are supported by CMurphi, Rumur
supports assumptions with the keyword ``assume``. Any state that fails an
assumption is considered irrelevant and discarded. You can read more about this
in properties.rst_.

.. _properties.rst: properties.rst

Liveness
--------
Another addition to the properties supported by CMurphi, is the Rumur-specific
``liveness``. From every state that is reached during checking, there must be a
state reachable from it that satisfies the liveness property. You can read more
about this in properties.rst_.

Comparisons
-----------
CMurphi has a limitation that ``=`` and ``!=`` can only be used to compare
simple values (enums, ranges or scalarsets). Rumur lets you compare any
compatible values; that is, it also supports records and arrays. The semantics
are equivalent to writing out a long form comparison of every one of the complex
values’ members.

Operators
---------
The operators ``&`` and ``|`` are used for logical AND and OR, respectively.
Rumur allows ``&&`` and ``||`` as synonyms for these. Rumur also supports a set
of bitwise arithmetic operators, documented in bitwise-operators.rst_.

.. _bitwise-operators.rst: ./bitwise-operators.rst

Command-line Options
--------------------
CMurphi has a set of command-line options, and its generated verifier has
another set of command-line options. Rumur has some similar command-line
options, but its generated verifier has none at all. The intent with this design
is to expose as much information to the C compiler as possible. In some cases
this can make a significant difference by allowing your C compiler to more
aggressively optimise during compilation.

Deadlock
--------
The verifier can detect “deadlocks” in your state graph, where there are no
transitions that make progress. CMurphi considers a deadlock to have occurred
in any state that only has enabled transitions that lead back to itself. Rumur
has two deadlock modes that can be selected with the ``--deadlock-detection``
command line option: “stuttering” and “stuck”. Stuttering, the default,  matches
CMurphi’s definition. Stuck uses a weaker definition that considers a deadlock
to have occurred only when a state has *no* enabled transitions.

Loops
-----
CMurphi requires the step in a quantifier expression to be a generation-time
constant. I.e. in the code:

.. code-block:: murphi

  for i := l to u by s do
    ...
  end;

``s`` must have a known constant value when the verifier is being generated.
Rumur has no such restrictions. However steps that are ``0`` at generation-time
are rejected and steps that turn out to be ``0`` or incrementing in the wrong
direction at runtime will be raised as a runtime error by the verifier.

CMurphi considers an infinite loop to be a runtime error that the verifier
should be capable of detecting and notifying the user about. It has an iteration
bound (by default 1000), after which it will consider a loop to be
non-terminating and will raise an error.

Rumur trusts the user not to write an infinite ``while`` loop. If you write an
infinite loop, your verifier will run forever. You have been warned.

Colour Output
-------------
Rumur’s generated verifier attempts to imitate CMurphi’s verifier’s output to
smooth the transition for users, but by default Rumur colourises its output
using ANSI terminal sequences. This behaviour is controllable via command-line
options.
