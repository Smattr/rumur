#!/usr/bin/env bash

# Download and install Macports. For use in CI.

# Version of Macports to install
VERSION=2.6.4

if [ "$(uname)" != "Darwin" ]; then
  printf 'this script is only intended to run on macOS\n'
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
curl --retry 3 -O https://distfiles.macports.org/MacPorts/MacPorts-${VERSION}.tar.bz2

# Decompress
tar xf MacPorts-${VERSION}.tar.bz2
cd MacPorts-${VERSION}

# Configure and install
./configure
make
sudo make install
