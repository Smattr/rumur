Integration Tests
=================
This directory contains an integration test suite for Rumur. It is expected to
be run with the current working directory as your build output directory:

.. code-block:: sh

    cd /my/build/dir
    cmake /my/source/dir
    make
    /my/source/dir/tests/integration-tests.py

In the subdirectories of this directory are various test cases, each defined in
a JSON_ file. Each file is expected to be a dictionary containing at least the
entry ``model`` giving a relative path to a Murphi input file used for the test:

.. _JSON: https://www.json.org/

.. code-block:: json

    {
      "model":"my-model.m"
    }

All other entries in the test case's dictionary are optional. Other useful
entries are:

* ``compile``: a boolean value indicating whether to compile the generated
  checker. This defaults to ``true``. If you set this to ``false``, the model
  will also, by extension, not be run.
* ``cxxflags``: a list of C++ compiler flags to override the default of
  ``['-std=c++11']``. These are only used if ``compile`` is not set to
  ``false``.
* ``extra_cxxflags``: a list of C++ compiler flags to append to the existing
  flags. These are only used if ``compile`` is not set to ``false``.
* ``run``: a boolean value indicating whether to run the compiled checker. This
  defaults to ``true``. If you set ``compile`` to ``false``, this is forced to
  ``false`` as well.

Currently the test suite expects all enabled steps to succeed (exit code 0). In
the future, it is planned to extend the available JSON options to allow
expressing that a step is expected to fail or even output specific text.
