Performance Summary
===================
Rumur comfortably outperforms all other Murphi-based model checkers (as far as
I am aware). There is no single magic bullet by which it achieves this, but
rather a combination of the following techniques:

1. Multicore parallelism
2. Specialised data structures:

   a. Lock-free, insert-only seen state set
   b. Lock-free per-thread pending state queues

3. Bump-pointer, non-freeing state allocation

To learn more about any of these, read the source of
../rumur/resources/header.c. The above list should give you a good intuition of
what to expect to find in the source code.
