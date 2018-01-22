Rumur
=====
This project is currently under development. You should not expect anything to
work. More description coming soon...

Quickstart
----------
First you will need to have the following dependencies installed:

* Either GCC_ or Clang_
* CMake_

Then you can build this project using the standard CMake steps:

.. code-block:: sh

    mkdir build
    cd build
    cmake ../src
    make
    make install

To use a different C++ compiler or to change the target path to install files
to, change the options passed to ``cmake``. You should now have the following
available in the directory you installed to:

* bin/rumur: Tool for translating a Murphi model into a program that implements
  a model checker;
* lib/librumur.a: A library for building your own Murphi model tools; and
* include/rumur/: The API for the above library.

Comparison with CMurphi
-----------------------
At a glance, some differences between Rumur and CMurphi:

* CMurphi considers the operator ``==`` an error, while Rumur treats it as an
  alias of ``=`` to ease life for C/C++ programmers.
* Rumur does not support floating point numbers (``real`` data type in CMurphi)
  and there are no current plans to add such support.

Legal
-----
Everything in this repository is in the public domain. Use in any way you see
fit.

.. _CMake: https://cmake.org/
.. _Clang: https://clang.llvm.org/
.. _GCC: https://gcc.gnu.org/
