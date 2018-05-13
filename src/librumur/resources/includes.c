#define _GNU_SOURCE /* for PTHREAD_RECURSIVE_MUTEX_INITIALIZER... */
#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Tell xxHash it is being compiled into a standalone binary. */
#define XXH_PRIVATE_API 1
#define XXH_STATIC_LINKING_API 1
