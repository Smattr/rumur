#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __linux__
  #include <linux/version.h>
#endif

#ifdef __APPLE__
  #include <sandbox.h>
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

/* Tell xxHash it is being compiled into a standalone binary. */
#define XXH_PRIVATE_API 1
#define XXH_STATIC_LINKING_API 1
