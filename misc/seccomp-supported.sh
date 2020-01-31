#!/usr/bin/env bash

# check whether seccomp sandboxing is supported on this platform
#
# This is useful for discriminating platforms where seccomp is unavailable or
# configured out.


# assume seccomp is unavailable on non-Linux platforms
if [ "$(uname -s)" != "Linux" ]; then
  exit 1
fi

# create a temporary space to work in
TMP=$(mktemp -d)
pushd ${TMP}

# create a sandbox testing program
cat - >test.c <<EOT
#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/syscall.h>

int main(void) {

  // disable addition of new privileges
  int r = prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);
  if (r != 0) {
    perror("prctl(PR_SET_NO_NEW_PRIVS) failed");
    return EXIT_FAILURE;
  }

  // a BPF program that allows everything
  static struct sock_filter filter[] = {

    // return allow
    BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_ALLOW),

  };

  static const struct sock_fprog filter_program = {
    .len = sizeof(filter) / sizeof(filter[0]),
    .filter = filter,
  };

  // apply the filter to ourselves
  r = prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &filter_program, 0, 0);
  if (r != 0) {
    perror("prctl(PR_SET_SECCOMP) failed");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
EOT

# compile the test program
if ! ${CC:-cc} -std=c11  test.c; then
  popd
  rm -rf "${TMP}"
  exit 1
fi

# execute the test program
if ! ./a.out; then
  popd
  rm -rf "${TMP}"
  exit 1
fi

# clean up
popd
rm -rf "${TMP}"

exit 0
