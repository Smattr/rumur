#!/usr/bin/env bash

# Test the Debian packaging infrastructure (../debian/)

# Reminder about updating ../debian/changelog: Pass --date=rfc2822 to git-log to
# get the commit dates in the right format for Debian packaging.

if [[ "$(uname -v)" != *"Debian"* ]]; then
  printf 'This script is only intended to run on Debian\n' >&2
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
