Rumur
=====
*This project is currently under development. You should not expect anything to
work. More description coming soon...*

Rumur is an explicit state `model checker`_ based on a previous tool, CMurphi_.
It takes the same input format, the Murphi modelling language, with some
extensions and produces C code implementing a verifier.

Quickstart
----------
First you will need to have the following dependencies installed:

* Either GCC_ or Clang_
* Bison_
* CMake_
* Flex_
* Libgmp_

Then:

.. code-block:: sh

    # Build Rumur
    mkdir build
    cd build
    cmake ..
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
Everything in this repository is in the public domain.

.. _Bison: https://www.gnu.org/software/bison/
.. _CMake: https://cmake.org/
.. _CMurphi: http://mclab.di.uniroma1.it/site/index.php/software/18-cmurphi
.. _Clang: https://clang.llvm.org/
.. _Flex: https://github.com/westes/flex
.. _GCC: https://gcc.gnu.org/
.. _Libgmp: https://gmplib.org/
.. _`model checker`: https://en.wikipedia.org/wiki/Model_checking
