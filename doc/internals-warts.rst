Internals — Warts
=================
This document covers some design flaws in Rumur that it would be nice to find a
solution to at some point.

Non-minimal traces when running multithreaded
---------------------------------------------
* `Github issue #131 “minimal trace mode”`_

Rumur defaults to generating a multithreaded verifier (``--threads 0``) that
aborts as soon as it finds an error (``--max-errors 1``). This was seen to be
pragmatic and aligning with what users typically want to do when bringing up a
new model. That is, run as fast as possible and stop as soon as we know an
exhaustive proof is impossible.

When examining counterexample traces, you generally want to be presented with
the shortest possible example. This happens naturally in a single-threaded
verifier as it performs a breadth-first search. The multithreaded verifier is
only approximately breadth-first because all the threads are essentially racing
with each other in their exploration. As a result, the first problem found may
not be the shallowest and may result in a non-minimal counterexample trace.

Each thread encounters states in greater-than-or-equal-depth order, so a naive
attempt at forcing a minimal trace is to temporarily stall a thread when it
finds an error. We can unblock this thread and allow it to report its error when
all other threads have either (a) finished exploration or (b) exceeded the depth
of the error location found by the first thread. If a second thread finds an
error at a shallower depth than the first, it can simply kill the first thread
and then itself stall as described.

Alas, this is insufficient. Taking the example from the Github issue, the state
machine may look as follows:

.. code-block::

  A -> B -> C -> ...
   \        ^
    \______/

Thread 0 might discover ``C`` first via the path ``A -> B -> C``. Later thread 1
might rediscover ``C`` via the path ``A -> C``. Thread 1 will notice its ``C``
is a duplicate and discard it. Now suppose one of the threads finds a deeper
error whose counterexample traces back to ``C``. This counterexample will trace
back through the ``A -> B`` prefix rather than the shorter ``A``.

So far I have not come up with a design that would result in a minimal
counterexample trace in a multithreaded verifier without degrading current
performance. Solving this would, I believe, be of significant value to users.

.. _`Github issue #131 “minimal trace mode”`: https://github.com/Smattr/rumur/issues/131

Incomplete AST in recursive functions
-------------------------------------
The librumur Abstract Syntax Tree (AST) represents child nodes using smart
pointers, ``rumur::Ptr``. This has constructor and assignment operator
implementations that allow the programmer to avoid thinking too much about
memory management. The general pattern is to freely copy the pointed-to object
into a new ``rumur::Ptr``. This isn’t very efficient, but has the advantage of
being simple. Moreover, there is little point tightly optimising AST
manipulation when any AST that is large enough to see benefit from this most
likely results from a model that is too large to verify.

A side-effect of this “deep copy” pointer design is that the AST is inherently a
Directed Acyclic Graph (DAG). It is not possible for an AST node to contain a
reference to something above itself in the AST because assigning this reference
takes a copy of the parent node.

This means that during symbol resolution, a recursive function call has no way
to store a fully resolved reference to its callee. To see why this is, follow
through the symbol resolution process. To begin with, the containing function
has a statement that includes an unresolved function call. The symbol resolution
traversal finds this unresolved function call and resolves it to the containing
function by copying it. However, this copy still contains the unresolved version
of the function call as a child. We can descend into the callee function
(``rumur::FunctionCall::function``) but we’ll merely repeat the process and have
a deeper unresolved function call.

The current design merely avoids descending into the callees of function calls.
This is acceptable, but it leaves an AST containing unresolved function calls
that may come as a surprise to later consumers.

Obtaining a fully resolved AST while supporting recursion seems inherently at
odds with the current design. It seems like it would be a net loss to permit
cycles in the AST (we would have to manually manage pointers and memory) but
perhaps there is another compromise we could find.
