#!/usr/bin/env bash

# Download and install AFL. For use in CI.

if [ -z "${CI}" ]; then
  printf 'CI variable not set. Set it and re-run if you really meant this to execute.\n' >&2
  exit 1
fi

# exit on error
set -e

# echo all commands
set -x

# create a temporary space to work in
TMP=$(mktemp -d)
pushd "${TMP}"

# download
curl --retry 3 -O http://lcamtuf.coredump.cx/afl/releases/afl-latest.tgz

# decompress
tar xf afl-latest.tgz
rm afl-latest.tgz
cd afl*

# compile
make

# install
sudo make install

# clean up
popd
rm -rf "${TMP}"
