Release Checklist
=================
The process of cutting a new Rumur release should follow these steps:

1. Update ``LAST_RELEASE`` in ../rumur/src/make-version.py. For version numbers,
   we use "vYYYY.MM.DD" with the date of the release.
2. Update ../CHANGELOG.rst with information about the changes in the new
   release.
3. Commit this to master.
4. Push this to `upstream on Github`_.
5. Wait for the `Travis CI regression tests`_ to pass. Travis is not very
   reliable and many errors are caused by infrastructure failures rather than
   your actual changes. So if you get a failure, check the logs to make sure
   it's not a false positive.
6. Tag the commit with the version number in "vYYYY.MM.DD" format.
7. Push the new version tag upstream.

Github's automated release process should notice the version tag and show the
new release as a downloadable zip/tarball on the "releases" tab of
https://github.com/Smattr/rumur.

.. _`upstream on Github`: https://github.com/Smattr/rumur
.. _`Travis CI regression tests`: https://travis-ci.org/Smattr/rumur/builds/
