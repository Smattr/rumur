#!/usr/bin/env bash

# wrapper around American Fuzzy Lop for use in CI (see ../.travis.yml)

# echo commands
set -x

# quit if we succeed exhaustively fuzzing (unlikely)
export AFL_EXIT_WHEN_DONE=1

# suppress normal animated UI and use periodic text output
export AFL_NO_UI=1

# run AFL fuzzing for 40m
timeout --preserve-status 2400s afl-fuzz -m 8192 -i ../tests -o findings_dir -- rumur --output /tmp/model.c @@

# remove the crashes dir if it is empty; this will fail if not empty
rmdir findings_dir/crashes &>/dev/null

# if we did not find any crashes, we are done
if [ ! -e findings_dir/crashes ]; then
  printf 'no crashes\n'
  exit 0
fi

# display information about any crashing models
find findings_dir/crashes -type f -exec printf 'crash in %s:\n' "{}" \; -exec cat "{}" \;

exit 1
