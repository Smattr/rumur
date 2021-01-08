#!/usr/bin/env bash

# CI script for installing build dependencies

# fail on error
set -e

# echo commands
set -x

# Debian
if which apt-get &>/dev/null; then

  apt-get update

  if ! which cmake &>/dev/null then
    apt-get install cmake
  fi
else
  printf 'no supported package manager found\n' >&2
  exit 1
fi
