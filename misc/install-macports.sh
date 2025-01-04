#!/usr/bin/env bash

# Download and install Macports. For use in CI.

# Version of Macports to install
VERSION=2.9.3

if [ "$(uname)" != "Darwin" ]; then
  printf 'this script is only intended to run on macOS\n' >&2
  exit 1
fi

# Exit on error
set -e

# Echo all commands
set -x

# Create a temporary space to work in
TMP=$(mktemp -d)
cd "${TMP}"

# Download
curl --retry 3 --location --no-progress-meter -O \
  https://github.com/macports/macports-base/releases/download/v${VERSION}/MacPorts-${VERSION}-14-Sonoma.pkg

# Install
sudo installer -package MacPorts-${VERSION}-14-Sonoma.pkg -target /
