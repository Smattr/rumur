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
should run on any POSIX operating system. However, the verifier's code does make
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

Type System
-----------
CMurphi supports real arithmetic using the ``real`` data type. Rumur does not
support this type and there are no plans to implement this or any floating point
support.

Rumur does not currently support the ``union`` type. This is planned to be added
in a future version.

While Statement Termination
---------------------------
CMurphi considers an infinite loop to be a runtime error that the verifier
should be capable of detecting and notifying the user about. It has an iteration
bound (by default 1000), after which it will consider a loop to be
non-terminating and will raise an error.

Rumur trusts the user not to write an infinite loop. If you write an infinite
loop, your verifier will run forever. You have been warned.
