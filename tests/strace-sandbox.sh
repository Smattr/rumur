#!/usr/bin/env bash

# run a sandboxed checker under strace
#
# When the Linux seccomp sandbox causes the checker to terminate because it made
# an unauthorised system call, it is difficult to debug what happened without
# stracing the process to see the denied system call. This test case automates
# this work flow. If the checker runs fine with the sandbox enabled, this test
# case is irrelevant. However, if the basic-sandbox.m test fails, this test will
# hopefully automatically diagnose the failure. The purpose of this existing
# within the test suite itself is to debug failures that occur within a CI
# environment you do not have access to or cannot easily replicate, like the
# Debian auto builders.

# check we have strace
if ! which strace &>/dev/null; then
  printf 'strace not available\n'
  exit 125
fi

# echo commands
set -x

# create a temporary space to work in
TMP=$(mktemp -d)
pushd ${TMP}

# create a basic model
cat - >model.m <<EOT
var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;
EOT

# generate a sandboxed checker
rumur --sandbox on --output model.c model.m

# check if the compiler supports -mcx16
cat - >mcx16-check.c <<EOT
int main(void) {
  return 0;
}
EOT
if ${CC:-cc} -std=c11 -mcx16 mcx16-check.c -o /dev/null; then
  MCX16=-mcx16
else
  MCX16=
fi

# check if we need libatomic support
cat - >libatomic-check.c <<EOT
#include <stdint.h>

int main(void) {
#if __SIZEOF_POINTER__ <= 4
  uint64_t target = 0;
#elif __SIZEOF_POINTER__ <= 8
  unsigned __int128 target = 0;
#else
  return (int)__sync_val_compare_and_swap(&target, 0, 1);
}
EOT
if ${CC:-cc} -std=c11 ${MCX16} libatomic-check.c -o /dev/null; then
  LIBATOMIC=
else
  LIBATOMIC=-latomic
fi

# compile the sandboxed checker
${CC:-cc} -stc=c11 ${MCX16} model.c -o model.exe ${LIBATOMIC} -lpthread

# run the model under strace
strace ./model.exe
RET=$?

# clean up
popd
rm -rf ${TMP}

exit ${RET}
