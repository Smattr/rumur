Internals — Atomics
===================
When writing multithreaded C code, there are four commonly used APIs for atomic
operations:

1. `inline assembly`_;
2. `__sync built-ins`_;
3. `__atomic built-ins`_; or
4. `C11 atomics`_

.. _`inline assembly`: https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html
.. _`__sync built-ins`: https://gcc.gnu.org/onlinedocs/gcc/_005f_005fsync-Builtins.html
.. _`__atomic built-ins`: https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html#g_t_005f_005fatomic-Builtins
.. _`C11 atomics`: https://en.cppreference.com/w/c/atomic

Rumur needs to use one of the above for each of its atomic operations. Inline
assembly is essentially a non-starter. It is non-portable and awkward. Of the
remaining three, they do not offer the equivalent functionality as seen from the
table below.

+----------------------------------+---------------------------------+------------------------------------+
| __sync built-ins                 | __atomic built-ins              | C11 atomics                        |
+==================================+=================================+====================================+
|                                  | ``__atomic_load_n``             | ``atomic_load``                    |
|                                  | ``__atomic_load``               |                                    |
+----------------------------------+---------------------------------+------------------------------------+
|                                  | ``__atomic_store_n``            | ``atomic_store``                   |
|                                  | ``__atomic_store``              |                                    |
+----------------------------------+---------------------------------+------------------------------------+
|                                  | ``__atomic_exchange_n``         | ``atomic_exchange``                |
|                                  | ``__atomic_exchange``           |                                    |
+----------------------------------+---------------------------------+------------------------------------+
| ``__sync_bool_compare_and_swap`` | ``__atomic_compare_exchange_n`` | ``atomic_compare_exchange_strong`` |
| ``__sync_val_compare_and_swap``  | ``__atomic_compare_exchange``   | ``atomic_compare_exchange_weak``   |
+----------------------------------+---------------------------------+------------------------------------+
| ``__sync_lock_test_and_set``     | ``__atomic_test_and_set``       | ``atomic_flag_test_and_set``       |
| ``__sync_lock_release``          | ``__atomic_clear``              | ``atomic_flag_clear``              |
+----------------------------------+---------------------------------+------------------------------------+
| ``__sync_fetch_and_add``         | ``__atomic_fetch_add``          | ``atomic_fetch_add``               |
| ``__sync_add_and_fetch``         | ``__atomic_add_fetch``          |                                    |
+----------------------------------+---------------------------------+------------------------------------+
| ``__sync_fetch_and_sub``         | ``__atomic_fetch_sub``          | ``atomic_fetch_sub``               |
| ``__sync_sub_and_fetch``         | ``__atomic_sub_fetch``          |                                    |
+----------------------------------+---------------------------------+------------------------------------+
| ``__sync_fetch_and_or``          | ``__atomic_fetch_or``           | ``atomic_fetch_or``                |
| ``__sync_or_and_fetch``          | ``__atomic_or_fetch``           |                                    |
+----------------------------------+---------------------------------+------------------------------------+
| ``__sync_fetch_and_and``         | ``__atomic_fetch_and``          | ``atomic_fetch_and``               |
| ``__sync_and_and_fetch``         | ``__atomic_and_fetch``          |                                    |
+----------------------------------+---------------------------------+------------------------------------+
| ``__sync_fetch_and_xor``         | ``__atomic_fetch_xor``          | ``atomic_fetch_xor``               |
| ``__sync_xor_and_fetch``         | ``__atomic_xor_fetch``          |                                    |
+----------------------------------+---------------------------------+------------------------------------+
| ``__sync_fetch_and_nand``        | ``__atomic_fetch_nand``         |                                    |
| ``__sync_nand_and_fetch``        | ``__atomic_nand_fetch``         |                                    |
+----------------------------------+---------------------------------+------------------------------------+

Some relevant points to note from this table and from other sources:

* The __sync built-ins have no way of expressing that a load or store must be
  atomic. This is not an issue on platforms like x86 where all naturally aligned
  loads and stores are guaranteed atomic, but it is a problem for other
  platforms.
* The __sync built-ins have no way of expressing an atomic exchange operation.
* The __sync built-ins have no way of expressing memory ordering. This is not an
  issue on platforms like x86 that guarantee sequential consistency, but on
  platforms that allow weaker guarantees it is an issue.
* Some of the C11 atomics expect to operate on ``atomic_`` types and are not
  directly usable on regular variables.
* Double-word compare and exchange using either the __atomic built-ins or the
  C11 atomics causes a call to libatomic to be emitted by GCC. This is
  functionally correct but impairs our ability to use these in lock-free
  algorithms. See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80878 and
  https://gcc.gnu.org/pipermail/gcc-help/2017-June.txt for more information.
* The C11 atomics are not available prior to GCC 4.9.

With these constraints in mind, the checker that Rumur generates mostly uses
__atomic built-ins, resorting to the __sync built-ins when doing something that
is not possible with the __atomic built-ins.
