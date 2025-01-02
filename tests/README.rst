Integration Tests
=================
This directory contains an integration test suite for Rumur. It is expected to
be run with Pytest:

.. code-block:: sh

    python3 -m pytest --capture=no --verbose run-tests.py

Within this directory are various test cases, each defined in a .m file. It is
possible to tweak the expected outcome of a test using specially formatted
comments in the source of a test case. E.g. to indicate that Rumur is expected
to reject a test case:

.. code-block:: murphi

    -- rumur_exit_code: 1

To see how these comments are interpreted and gain a full understanding of what
is possible, consult the test script run-tests.py.
