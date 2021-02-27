/* Setting a POSIX version on FreeBSD causes other functions to become hidden.
 */
#ifndef __FreeBSD__
#define _POSIX_C_SOURCE 200809L
#endif

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __linux__
#include <linux/version.h>
#endif

#ifdef __APPLE__
#include <sandbox.h>
#elif defined(__FreeBSD__)
#include <sys/capsicum.h>
#elif defined(__linux__)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0)
#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#endif
#endif
