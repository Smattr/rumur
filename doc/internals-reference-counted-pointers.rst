Internals — Reference Counted Pointers
======================================
The generated verifier contains an implementation of reference counted pointers,
``refcounted_ptr``. This document describes their design and how they work.

Definition
----------

.. code-block:: c

    struct refcounted_ptr {
      void *ptr;
      size_t count;
    };

A reference counted pointer contains two fields, the raw pointer and a count of
the number of outstanding (i.e. live) references. You can think of the count as
the number of threads who have "borrowed" a copy of this pointer.

Interface
---------
The basic operations on a reference counted pointer are get and put:

.. code-block:: c

    void *refcounted_ptr_get(struct refcounted_ptr *p);
    size_t refcounted_ptr_put(struct refcounted_ptr *p, void *ptr);

The get operation takes a reference to the pointer (increments its ``count`` by
``1``) and returns the raw pointer value. The caller can freely use this
pointer as it would any other pointer.

The put operation returns a reference to the pointer (decrements its ``count``
by ``1``) and returns the number of remaining references to this pointer. If the
returned value is ``0``, the caller can free the raw pointer as it knows there
are no other remaining users of it.

There is nothing to prevent another thread calling ``refcounted_ptr_get`` on a
pointer whose reference count has just dropped to ``0`` and thereby increasing
it back to ``1``. That is, after calling ``refcounted_ptr_put`` and receiving
``0`` as the return value, it is possible for another thread to increase the
reference count prior to the pointer being freed. It is up to threads to
coordinate their actions to avoid this race.

The ``ptr`` parameter to ``refcounted_ptr_put`` is for debugging purposes only.

Two other operations are provided for setting the value of a reference counted
pointer:

.. code-block:: c

    void refcounted_ptr_set(struct refcounted_ptr *p, void *ptr);
    void refcounted_ptr_shift(struct refcounted_ptr *current, struct refcounted_ptr *next);

The first of these, ``refcounted_ptr_set``, sets the pointer's raw value to
``ptr`` and zeroes its reference count. The second, ``refcounted_ptr_shift``,
replaces fields in ``current`` with those in ``next`` and zeroes ``next``.
Neither of these functions use atomic operations and threads are expected to
coordinate with each other such that calls to these do not race with each other
or with gets and puts.

Implementation of Atomic Updates
--------------------------------
Both ``refcounted_ptr_get`` and ``refcounted_ptr_put`` need to operate
atomically on the pointer they are affecting. This is straightforward for put
that only needs to atomically decrement the reference count, but get needs to
read the pointer and the count atomically while incrementing the count.

To achieve this, we use a double-word compare-exchange operation. First we read
the entire structure atomically. We then extract the pointer value and increment
the count in the local copy we have. Finally we use an atomic compare-exchange
to write the changes back to the original structure. This relies on platform
support for atomic double-word compare-exchange. For example, on x86-64 this
involves a ``CMPXCHG16B`` instruction.
