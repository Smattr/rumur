Release Checklist
=================
The process of cutting a new Rumur release should follow these steps:

1. Update ``LAST_RELEASE`` in ../rumur/src/make-version.py. For version numbers,
   we use "vYYYY.MM.DD" with the date of the release.
2. Update ../CHANGELOG.rst with information about the changes in the new
   release. Changes should be separated into "user-facing" and "internal,"
   providing commit hash(es) as a reference where possible. The ordering in
   which changes are listed should firstly prioritise bug fixes (which should be
   explicitly marked as such) and then by the impact on users/developers a
   change will have. Code changes that are only of relevance to people hacking
   on Rumur can be omitted, and this audience can read about them in the Git
   log.
3. Commit this to master.
4. Push this to `upstream on Github`_.
5. Wait for the `Travis CI regression tests`_ to pass. Travis is not very
   reliable and many errors are caused by infrastructure failures rather than
   your actual changes. So if you get a failure, check the logs to make sure
   it is not a false positive.
6. Wait for the `Cirrus CI FreeBSD tests`_ to pass. It is important for the new
   release to work on FreeBSD because Rumur is in
   `FreeBSD’s package repository`_ and new releases are pulled in automatically.
7. Tag the commit with the version number in "vYYYY.MM.DD" format.
8. Push the new version tag upstream.
9. Package Rumur for Debian (see below).

Github’s automated release process should notice the version tag and show the
new release as a downloadable zip/tarball on the "releases" tab of
https://github.com/Smattr/rumur.

Packaging for Debian
--------------------
Rumur is currently `packaged in Debian unstable`_. To update the Debian Rumur
package follow these steps.

1. Switch to the branch packaging/debian.
2. Merge from master.
3. Update the Debian changelog (../debian/changelog). Debian provide guidance on
   the `changelog format`_.
4. Update ../debian/rules to set ``RUMUR_VERSION`` to the new version. Note that
   Debian version numbers are the standard ("native") Rumur version with "-1"
   appended.
5. Commit these changes.
6. Run ../misc/package-for-debian.sh to prepare a new Debian package for upload.
7. Follow further instructions in ../misc/package-for-debian.sh for testing,
   signing, and uploading the resulting package.

.. _`changelog format`: https://www.debian.org/doc/manuals/maint-guide/dreq.en.html#changelog
.. _`Cirrus CI FreeBSD tests`: https://cirrus-ci.com/github/Smattr/rumur
.. _`FreeBSD’s package repository`: https://svnweb.freebsd.org/ports/head/math/rumur/
.. _`packaged in Debian unstable`: https://packages.debian.org/sid/rumur
.. _`upstream on Github`: https://github.com/Smattr/rumur
.. _`Travis CI regression tests`: https://travis-ci.org/Smattr/rumur/builds/
