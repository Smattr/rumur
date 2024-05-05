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

# check whether sandboxing is available
if [ "${HAS_SANDBOX}" != "True" ]; then
  printf 'seccomp sandboxing not supported\n'
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

# check if the compiler supports -march=native
if [ "${HAS_MARCH_NATIVE}" = "True" ]; then
  MARCH=-march=native
else
  MARCH=
fi

# check if the compiler supports -mcx16
if [ "${HAS_MCX16}" = "True" ]; then
  MCX16=-mcx16
else
  MCX16=
fi

# check if we need libatomic support
if [ "${NEEDS_LIBATOMIC}" != "False" ]; then
  LIBATOMIC=-latomic
else
  LIBATOMIC=
fi

# compile the sandboxed checker
${CC:-cc} -std=c11 ${MARCH} ${MCX16} model.c -o model.exe ${LIBATOMIC} -lpthread

# run the model under strace
strace ./model.exe
RET=$?

# if we did not yet see a failure, test to see if the sandbox also permits
# anything extra we do with --debug
if [ $RET -eq 0 ]; then

  # generate a sandboxed checker with debugging enabled
  rumur --sandbox on --debug --output model.c model.m

  # compile the sandboxed checker
  ${CC:-cc} -std=c11 ${MARCH} ${MCX16} model.c -o model.exe ${LIBATOMIC} -lpthread

  # run the model under strace
  strace ./model.exe
  RET=$?

fi

# clean up
popd
rm -rf ${TMP}

exit ${RET}
