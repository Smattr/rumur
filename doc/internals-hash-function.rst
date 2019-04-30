Internals — Hash Function
=========================
The seen state set (described in `internals-seen-state-set.rst`_) hashes the
contents of state data to determine an index at which to store that state. The
hash function used for this is MurmurHash_ from `Austin Appleby`_. This hash
function was chosen because it has reasonable statistical properties and is
public domain. There might be a performance benefit to switching to a different
hash function, but so far this has not proved a bottleneck.

.. _`Austin Appleby`: https://github.com/aappleby
.. _`internals-seen-state-set.rst`: ./internals-seen-state-set.rst
.. _MurmurHash: https://github.com/aappleby/smhasher
