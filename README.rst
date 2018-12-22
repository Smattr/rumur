Rumur
=====
Rumur is a `model checker`_, a formal verification tool for proving safety and
security properties of systems represented as state machines. It is based on a
previous tool, CMurphi_, and intended to be close to a drop-in replacement.
Rumur takes the same input format as CMurphi, the Murphi modelling language,
with some extensions and generates a C program that implements a verifier.

Quickstart
----------
First you will need to have the following dependencies installed:

* Either GCC_ or Clang_
* Bison_
* CMake_
* Flex_
* Libgmp_
* Python_

Then:

.. code-block:: sh

    # Download Rumur
    git clone https://github.com/Smattr/rumur

    # Configure and compile
    mkdir build
    cd build
    cmake ../rumur
    make
    make install

    # Generate a checker
    rumur my-model.m --output my-model.c

    # Compile the checker (also pass -mcx16 if using GCC on x86-64)
    cc -std=c11 -O3 my-model.c -lpthread

    # Run the checker
    ./a.out

Compilation produces several artefacts including the `rumur` binary itself:

* bin/rumur: Tool for translating a Murphi model into a program that implements
  a checker;
* bin/rumur-ast-dump: Tool for emitting an XML representation of a Murphi
  model's Abstract Syntax Tree;
* lib/librumur.a: A library for building your own Murphi model tools; and
* include/rumur/: The API for the above library.

Comparison with CMurphi
-----------------------
If you are migrating from CMurphi, you can read a comparison between the two
model checkers at `doc/vs-cmurphi.rst`_.

.. _doc/vs-cmurphi.rst: doc/vs-cmurphi.rst

Legal
-----
Everything in this repository is in the public domain, under the terms of
`the Unlicense`_. For the full text, see `LICENSE`_.

.. _Bison: https://www.gnu.org/software/bison/
.. _CMake: https://cmake.org/
.. _CMurphi: http://mclab.di.uniroma1.it/site/index.php/software/18-cmurphi
.. _Clang: https://clang.llvm.org/
.. _Flex: https://github.com/westes/flex
.. _GCC: https://gcc.gnu.org/
.. _Libgmp: https://gmplib.org/
.. _`model checker`: https://en.wikipedia.org/wiki/Model_checking
.. _Python: https://www.python.org/
.. _`the Unlicense`: http://unlicense.org/
