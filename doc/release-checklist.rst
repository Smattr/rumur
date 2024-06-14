Release Checklist
=================
The process of cutting a new Rumur release should follow these steps:

1. Grep the source tree for any deprecated functions or data (marked with
   ``__attribute__((deprecated))`` or a “deprecated” comment). Any that were
   deprecated more than six months ago can, and should, be removed for the
   upcoming release.
2. Check the `Debian buildd results`_ for the last uploaded version of Rumur.
   If there were any failures, consider introducing commits to try to address
   them prior to release. The buildd tests are only run each time there is a
   new Debian package uploaded, so the turn around time on seeing a failure
   here and having an opportunity to fix it can be long.
3. Update ../CHANGELOG.rst with information about the changes in the new
   release. Changes should be separated into “user-facing” and “internal,”
   providing commit hash(es) as a reference where possible. The ordering in
   which changes are listed should firstly prioritise bug fixes (which should
   be explicitly marked as such) and then by the impact on users/developers a
   change will have. Code changes that are only of relevance to people hacking
   on Rumur can be omitted, and this audience can read about them in the Git
   log.
4. Commit this to main.
5. Push this to `upstream on Github`_.
6. Wait for the `Cirrus CI regression tests`_ to pass. If one of these tests
   fail, you may need to look at the raw log because the summary output hides
   some stderr lines.
7. Tag the commit with the version number in “vYYYY.MM.DD” format.
8. Push the new version tag upstream.
9. Package Rumur for Debian (see below).

Github’s automated release process should notice the version tag and show the
new release as a downloadable zip/tarball on the “releases” tab of
https://github.com/Smattr/rumur.

Packaging for Debian
--------------------
Rumur is currently `packaged in Debian`_. To update the Debian Rumur package
follow these steps.

1. Switch to the branch packaging/debian.
2. Merge from main.
3. Update the Debian changelog (../debian/changelog). Debian provide guidance on
   the `changelog format`_.
4. Commit these changes.
5. Push this upstream to packaging/debian.

Then on a `Debian Unstable installation`_:

.. code-block:: sh

  # 6. Upgrade all packages.
  sudo apt update
  sudo apt upgrade

  # 7. Clone the packaging/debian branch.
  git clone https://github.com/Smattr/rumur -b packaging/debian
  cd rumur

  # 8. Run packaging script.
  ./misc/package-for-debian.sh

  # If any lintian errors or warnings were output, you will need to address
  # these and then return to step 4.

  # 9. Update pbuilder environment. You will need to already have a pbuilder
  # environment created, as described in misc/package-for-debian.sh.
  sudo pbuilder update

  # 10. Test the package under pbuilder
  pdebuild

  # 11. Sign the source package that pbuilder created. You will need GPG key(s)
  # configured.
  debsign ../rumur_*_source.changes

  # 12. Upload to the mentors inbox. You will need dput configured, as described
  # in misc/package-for-debian.sh.
  dput mentors ../rumur_*_source.changes

In a few minutes you should get a confirmation email that the upload succeeded.
Then:

13. Follow the instructions included in the confirmation email to send a
    `Request For Sponsorship`_ to the Debian Mentors mailing list. Hope that you
    get a reply from an interested party. Make sure “Needs a sponsor” is set to
    “Yes” on the `package details page`_.

14. When/if the package is accepted into Debian unstable, tag the commit used to
    produce it with the Debian version number in “debian/vYYYY.MM.DD-1” format.
    Push this upstream.

Note that there could still be problems with the package that a sponsor may
request you fix. For example, there is currently no easy way to smoke test the
autopkgtests and these could fail, preventing migration of the package to the
main repositories.

.. _`changelog format`: https://www.debian.org/doc/manuals/maint-guide/dreq.en.html#changelog
.. _`Cirrus CI regression tests`: https://cirrus-ci.com/github/Smattr/rumur
.. _`Debian buildd results`: https://buildd.debian.org/status/package.php?p=rumur
.. _`Debian Unstable installation`: https://wiki.debian.org/DebianUnstable#Installation
.. _`package details page`: https://mentors.debian.net/package/rumur/
.. _`packaged in Debian`: https://packages.debian.org/bullseye/rumur
.. _`Request For Sponsorship`: https://mentors.debian.net/sponsors/rfs-howto
.. _`upstream on Github`: https://github.com/Smattr/rumur
