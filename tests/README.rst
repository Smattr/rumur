Integration Tests
=================
This directory contains an integration test suite for Rumur. It is expected to
be run with the current working directory as your build output directory:

.. code-block:: sh

    cd /my/build/dir
    cmake /my/source/dir
    make
    /my/source/dir/tests/run-tests.py

Within this directory are various test cases, each defined in a .m file. It is
possible to tweak the expected outcome of a test using specially formatted
comments in the source of a test case. E.g. to indicate that Rumur is expected
to reject a test case:

.. code-block:: murphi

    -- rumur_exit_code: 1

To see how these comments are interpreted and gain a full understanding of what
is possible, consult the test script run-tests.py.
