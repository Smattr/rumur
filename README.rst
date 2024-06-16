Rumur
=====
Rumur is a `model checker`_, a formal verification tool for proving safety and
security properties of systems represented as state machines. It is based on a
previous tool, CMurphi_, and intended to be close to a drop-in replacement.
Rumur takes the same input format as CMurphi, the Murphi modelling language,
with some extensions and generates a C program that implements a verifier.

A more extended introduction is available in `doc/introduction.rst`_

.. _`doc/introduction.rst`: doc/introduction.rst

Quickstart
----------

Installation on Ubuntu or Debian
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: sh

  apt install rumur

Installation on FreeBSD
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: sh

  pkg install rumur

Thanks to `yuri@FreeBSD`_ for packaging.

.. _`yuri@FreeBSD`: https://github.com/yurivict

Building from Source
~~~~~~~~~~~~~~~~~~~~
First you will need to have the following dependencies installed:

* Either GCC_ or Clang_
* Bison_
* CMake_
* Flex_
* Libgmp_
* Python_ ≥ 3.4

Then:

.. code-block:: sh

  # Download Rumur
  git clone https://github.com/Smattr/rumur
  cd rumur

  # Configure and compile
  cmake -B build -S .
  cmake --build build
  cmake --install build

  # Generate a checker
  rumur my-model.m --output my-model.c

  # Compile the checker (also pass -mcx16 if using GCC on x86-64)
  cc -std=c11 -march=native -O3 my-model.c -lpthread

  # Run the checker
  ./a.out

Compilation produces several artefacts including the `rumur` binary itself:

* rumur: Tool for translating a Murphi model into a program that implements
  a checker;
* murphi2c: Tool for translating a Murphi model into C code for use in a
  simulator;
* murphi2murphi: A preprocessor for Murphi models;
* murphi2smv: Tool for translating a Murphi model into `NuSMV` input;
* murphi2uclid: Tool for translating a Murphi model into `Uclid5` input;
* murphi2xml: Tool for emitting an XML representation of a Murphi model’s
  Abstract Syntax Tree;
* librumur.a: A library for building your own Murphi model tools; and
* include/rumur/: The API for the above library.

Comparison with CMurphi
-----------------------
If you are migrating from CMurphi, you can read a comparison between the two
model checkers at `doc/vs-cmurphi.rst`_.

.. _doc/vs-cmurphi.rst: doc/vs-cmurphi.rst

Legal
-----
Everything in this repository is in the public domain, under the terms of
`the Unlicense`_. For the full text, see LICENSE_.

.. _Bison: https://www.gnu.org/software/bison/
.. _CMake: https://cmake.org/
.. _CMurphi: http://mclab.di.uniroma1.it/site/index.php/software/18-cmurphi
.. _Clang: https://clang.llvm.org/
.. _Flex: https://github.com/westes/flex
.. _GCC: https://gcc.gnu.org/
.. _Libgmp: https://gmplib.org/
.. _LICENSE: ./LICENSE
.. _`model checker`: https://en.wikipedia.org/wiki/Model_checking
.. _NuSMV: https://nusmv.fbk.eu/
.. _Python: https://www.python.org/
.. _`the Unlicense`: http://unlicense.org/
.. _Uclid5: https://github.com/uclid-org/uclid
