Standard Library
================
The Rumur Standard Library is a collection of fragments designed for reuse in
user models. They are written as M4_ templates, to be then included in other
models using pre-processing directives.

.. _M4: https://en.wikipedia.org/wiki/M4_(computer_language)

Locating the Standard Library
-----------------------------
Making the Standard Library available when pre-processing requires teaching M4
where the Standard Library itself lives. If Rumur has been installed the usual
way, you can find the Standard Library based on the path to the ``rumur``
binary:

.. code-block:: sh

  m4 --include=$(dirname $(which rumur))/../share/rumur/lib …

Using Standard Library Components
---------------------------------
Most fragments define macros that you can then invoke from your own code. For
example, to use ``list``:

.. code-block:: m4

  include(`v2025.02.01/list')dnl
  _list(`foo', `bar_t', `baz_t')dnl

Note that the example above includes the ``list`` template specifically from
Rumur v2025.02.01. It is good practice to reference a specific version of
standard library templates like this, to ensure your model remains runnable on
future versions of Rumur. If you are working on a model that does not need this
kind of stability, you can include the latest version of the template with
``include(`list')`` but this usage may break on Rumur upgrades.

Contributing to the Standard Library
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
If you have a general piece of functionality that you believe deserves to live
in the Rumur Standard Library, please feel free to submit it at
https://github.com/Smattr/rumur/pulls. All current Standard Library components
are public domain licensed. You can donate your own contribution to the public
domain or start a conversation with the Rumur maintainers about alternative
licensing of your contributions.

Technical Minutiae
~~~~~~~~~~~~~~~~~~
A commonly asked question is why M4 is used instead of the C Pre Processor
(``cpp``). ``cpp`` assumes the source it is processing is C/C++ and has several
behaviours that make it awkward to use on Murphi. These can be worked around,
and indeed many industrial users have successfully applied ``cpp`` to Murphi,
but this is a little like using a hammer on something that is not a nail. A more
flexible and robust approach is to use a generic pre-processor like M4 that does
not make assumptions about the concrete language being generated.

The use of ``dnl`` in examples above is an M4 technique for suppressing newlines
that would otherwise appear in the pre-processed output at the location of
template inclusion. It is an optional nicety, but generally a good habit to get
into.

M4 supports a way of changing the quoting characters (``changequote``). This is
not used in the Rumur Standard Library because the default quoting characters
conveniently do not conflict with Murphi syntax. Changing the quoting characters
in user models and having this change also apply to Standard Library templates
is not supported.

When pre-processing a model, Rumur has no knowledge of the originating files.
Thus it can be hard to debug typos, when Rumur’s error messages point to
locations in the pre-processed output. In future it may be desirable to
introduce something like the C Pre Processor’s ``#line`` directives to
mitigate this.
