Integration Tests
=================
This directory contains an integration test suite for Rumur. It is expected to
be run with the current working directory as your build output directory:

.. code-block:: sh

    cd /my/build/dir
    cmake /my/source/dir
    make
    /my/source/dir/tests/integration-tests.py

Within this directory are various test cases, each defined in a .m file.
Currently the test suite expects all steps to succeed (exit code 0). In
the future, it is planned to extend the available options to allow expressing
that a step is expected to fail or even output specific text.
