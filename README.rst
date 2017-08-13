Rumur
=====
This project is currently under development. You should not expect anything to
work. More description coming soon...

Quickstart
----------
First you will need to have the following dependencies installed:

* A C++ compiler with `C++17`_ support (recent GCC_ or Clang_)
* CMake_
* Ninja_ (you can get by with `GNU Make`_ but the experience will be worse)

Then you can build this project using the standard CMake steps:

.. code-block:: sh

    mkdir build
    cd build
    cmake -G Ninja ../src
    ninja
    ninja install

To use a different C++ compiler or to change the target path to install files
to, change the options passed to ``cmake``. You should now have the following
available in the directory you installed to:

* bin/rumur: Tool for translating a Murphi model into a program that implements
  a model checker;
* lib/librumur.a: A library for building your own Murphi model tools; and
* include/rumur/: The API for the above library.

Legal
-----
Everything in this repository is in the public domain. Use in any way you see
fit.

.. _C++17: https://en.wikipedia.org/wiki/C%2B%2B17
.. _CMake: https://cmake.org/
.. _Clang: https://clang.llvm.org/
.. _GCC: https://gcc.gnu.org/
.. _GNU Make: https://www.gnu.org/software/make/
.. _Ninja: https://ninja-build.org/
