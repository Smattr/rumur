#!/usr/bin/env bash

# CI script for installing build dependencies

if [ -z "${CI}" ]; then
  printf '$CI unset; refusing to run\n' >&2
  exit 1
fi

# fail on error
set -e

# echo commands
set -x

# Debian
if which apt-get &>/dev/null; then

  apt-get update

  if ! which cmake &>/dev/null; then
    env DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends cmake
  fi
else
  printf 'no supported package manager found\n' >&2
  exit 1
fi
