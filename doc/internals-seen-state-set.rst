Internals — Seen State Set
==========================
When exploring the state space of your model, the generated verifier stores the
states it has encountered in a global set structure. This structure's key 
properties are:

* Insert-only: there is no support for removing elements from the set.
* Thread-safe, lock-free: multiple threads can be inserting into the set at once
  without creating a scalability bottleneck.
* Dynamically expanding: when the set fills up, its capacity is expanded to hold
  more states.

High Level Design
-----------------
The seen state set is modelled closely after the data structure described in
Maier et al, "Concurrent Hash Tables: Fast and General(?)!" in arXiv 2016. This
paper lays the ground work for how to build a scalable concurrent set, and the
remainder of the present document will assume you have read and understood the
paper.

The main point of deviation from Maier et al's design is that we do not
implement the optimisation that allows their set expansion to use non-atomic
writes. In the Rumur design, each thread has a unique "chunk" (a 4KB block of
state pointers) that it is migrating, but the destinations for these elements
may collide with the writes of another thread. We use atomic exchanges to guard
against this interference.

The main reason we don't implement this optimisation is that I was initially
anticipating a configuration where you fully fill the seen set before expansion,
instead of expanding at a threshold like 60% occupancy. Now I see this as a less
realistic scenario, so it might be worth looking into this optimisation in
future.

Definition
----------
The set itself is quite simple:

.. code-block:: c

  struct set {
    slot_t *bucket;
    size_t size_exponent;
  };

The total capacity of the set can be computed by ``1 << size_exponent``,
encapsulated in the function ``set_size()``. We store an exponent rather than
the size itself as a micro-optimisation to make it explicit to the compiler that
the set capacity is always a power of two.

The occupancy of the set (how many elements are actually stored in the set) is
stored in a global:

.. code-block:: c

  static size_t seen_count;

The occupancy only grows throughout the program's lifetime and storing it in a
global (a) allows it to be referenced without a pointer indirection and (b)
decreases the size of reallocation by ``sizeof(size_t)`` when expanding the set.

Local and Global Pointers
-------------------------
During execution there is a single global pointer to the currently active seen
set and a local pointer to this set for each thread. The global pointer is
reference counted (see `internals-reference-counted-pointers.rst`_). The
motivation for this and mechanics of pointer borrowing and returning will become
clearer when we discuss set expansion.

.. _`internals-reference-counted-pointers.rst`: ./internals-reference-counted-pointers.rst

Set Insertion
-------------
The common path for inserting an element into the set is quite straightforward.
A thread calls the set insertion function:

.. code-block:: c

  bool set_insert(struct state *s, size_t *count);

Within the function, a hash function (described in
`internals-hash-function.rst`_) is used to compute the index at which the
element should be stored. Using a thread-local pointer to access the currently
active seen state set, the insertion algorithm uses `linear probing`_ to find an
empty slot to insert the state pointer into.

.. _`internals-hash-function.rst`: ./internals-hash-function.rst
.. _`linear probing`: https://en.wikipedia.org/wiki/Linear_probing

If you are following along in `../rumur/resources/header.c`_, you will have
noticed two odd things:

1. The insertion algorithm checks occupancy and conditionally calls
   ``set_expand()``; and
2. There is a check for something called a "tombstone."

Both of these are explained in the next section.

.. _`../rumur/resources/header.c`: ../rumur/resources/header.c

Set Expansion
-------------
When the set exceeds a pre-defined occupancy threshold, it is expanded by
doubling its size. A simplified summary of how this is done is:

1. ``malloc()`` a new set twice the size of the existing one.
2. Take each state stored in the old set, rehash it, and insert it into the new
   set.
3. ``free()`` the old set.

But how is this possible if the verifier is multithreaded and set insertion is
lock-free?

Firstly, when a thread decides to expand the seen state set it can either "win"
the race to allocate the new set or "lose" the race and fall back to joining a
set migration begun by a previous thread. During migration, threads grab
successive "chunks" (4KB blocks of state pointers) to exclusively migrate by
atomically incrementing a shared counter. As they migrate state pointers from
the old set to the new, they replace the old state pointer with a "tombstone"
value.

We can now understand why ``set_insert()`` is checking for tombstones. If it
sees a tombstone, it knows a set expansion and migration has been initiated by
another thread. It can then hold off on its insertion, join the migration
effort, then return to attempting its insertion on the new set.

At the end of a set migration, all threads synchronise at a rendezvous point (to
be described in forthcoming documentation). The last arriving thread to the
rendezvous point updates the global seen set pointer to point to the new set.
When threads depart the rendezvous point, they borrow a new local copy of this
pointer.

A Note on Complexity
--------------------
The seen state set is one of the most complex and performance sensitive
components of the verifier. Given I am not a professional technical writer, it
is unlikely you have fully understood how it works from the above description.
If you are interested in learning more, I encourage you to read the source in
`../rumur/resources/header.c`_. It is dense and the control flow can be counter
intuitive, but unfortunately I did not have more time to make it simpler.
