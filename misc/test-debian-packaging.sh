#!/usr/bin/env bash

# Test the Debian packaging infrastructure (../debian/)

# Some relevant notes for Debian packaging:
#   * Pass --date=rfc2822 to git-log to get the commit dates in the right format
#     for Debian packaging.
#   * After successful packaging, sign the release with
#     `debsign ../rumur_<version>_<arch>.changes`.
#   * Upload the package to mentors.debian.net with
#     `dput mentors ../rumur_<version>_<arch>.changes`.
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

if [ "$(lsb_release --release | cut -d "	" -f2)" != "unstable" ]; then
  printf 'This script is only intended to run on Debian unstable\n' >&2
  exit 1
fi

# Move to our parent directory
cd "$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)/.."

# Exit on error
set -e

# Echo all commands
set -x

# Build the package from scratch
dpkg-buildpackage -us -uc

# Check for consistency errors in the built package
lintian -i -I --show-overrides
