#!/usr/bin/env bash

# Generate contents of a version.cc.

# Exit on error
set -e

# Make any pipeline component's failure fail the whole pipeline
set -o pipefail

printf 'const char *VERSION = "'
git rev-parse --verify HEAD | tr -d '\n'
printf '";\n'
