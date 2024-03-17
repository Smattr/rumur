#!/usr/bin/env bash

# Package Rumur for Debian. To use this, you must be on the 'packaging/debian'
# branch.

# Some relevant notes for Debian packaging:
#   * Pass --date=rfc2822 to git-log to get the commit dates in the right format
#     for Debian packaging.
#   * After successful packaging, sign the release with
#     `debsign ../rumur_<version>_source.changes`.
#   * Upload the package to mentors.debian.net with
#     `dput mentors ../rumur_<version>_source.changes`.
#   * Check consistency of ../debian/watch with
#     `cd .. && uscan --report --verbose`.
#   * For building with pbuilder, first
#     `sudo pbuilder create --debootstrapopts --variant=buildd` to setup a base
#     chroot image, then `pdebuild` to test.
#   * To update a pre-existing pbuilder environment, `sudo pbuilder update`.
#
# For uploading to mentors.debian.net, you will need ~/.dput.cf configured:
#
#   [mentors]
#     fqdn = mentors.debian.net
#     incoming = /upload
#     method = https
#     allow_unsigned_uploads = 0
#     progress_indicator = 2
#     allowed_distributions = .*

if [ "$(uname -s)" != "Linux" ]; then
  printf 'This script is only intended to run on Linux (Debian unstable)\n' >&2
  exit 1
fi

if [ "$(lsb_release --id | cut -d "	" -f2)" != "Debian" ]; then
  printf 'This script is only intended to run on Debian unstable\n' >&2
  exit 1
fi

# Move to our parent directory
cd "$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)/.."

# check for artefacts from a prior build
ls ../rumur_* &>/dev/null
if [ $? -eq 0 ]; then
  printf 'There are files leftover from the previous build (../rumur_*)\n' >&2
  exit 1
fi

# check git tree
if [[ ! -z "$(git status --short)" ]]; then
  printf 'Working tree is not clean\n' >&2
  exit 1
fi

# Exit on error
set -e

# Echo all commands
set -x

# Build the package from scratch
gbp buildpackage --git-ignore-new --git-upstream-tag='v%(version)s' --git-debian-branch=packaging/debian

# Check for consistency errors in the built package
lintian --display-experimental --display-info --info --pedantic --show-overrides --verbose

printf 'Run pdebuild to test the package under pbuilder\n'
