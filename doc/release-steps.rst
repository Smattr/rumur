Release Steps
=============
The process of cutting a new Rumur release should follow these steps:

1. Update ``LAST_RELEASE`` in ../rumur/src/make-version.py. For version numbers,
   we use "vYYYY.MM.DD" with the date of the release.
2. Update ../CHANGELOG.rst with information about the changes in the new
   release.
3. Update the ``CHANGELOG`` section of ../rumur/doc/rumur.1 with a copy of the
   information from (2).
4. Commit this to master.
5. Tag the commit with the version number in "vYYYY.MM.DD" format.
6. Push both master and the new version tag upstream.

Github's automated release process should notice the version tag and show the
new release as a downloadable zip/tarball on the "releases" tab of
https://github.com/Smattr/rumur.
