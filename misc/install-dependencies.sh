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

function update() {
  # Debian
  if which apt-get &>/dev/null; then
    apt-get update
  # FreeBSD
  elif which pkg &>/dev/null; then
    pkg upgrade -y
  fi
}

function install() {
  # Debian
  if which apt-get &>/dev/null; then
    env DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends "${1}"
  elif
    pkg install -y "${1}"
  else
    printf 'no supported package manager found\n' >&2
    exit 1
  fi
}

# update package manager database
update

# install dependencies
install bison
install cmake
install flex
