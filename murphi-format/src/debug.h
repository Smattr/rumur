#pragma once

#include "compiler.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

extern bool debug;

/// emit a debug message
#define DEBUG(...)                                                             \
  do {                                                                         \
    if (UNLIKELY(debug)) {                                                     \
      const char *name_ = strrchr(__FILE__, '/');                              \
      flockfile(stderr);                                                       \
      fprintf(stderr, "[MURPHI-FORMAT] murphi-format/src%s:%d: ", name_,       \
              __LINE__);                                                       \
      fprintf(stderr, __VA_ARGS__);                                            \
      fprintf(stderr, "\n");                                                   \
      funlockfile(stderr);                                                     \
    }                                                                          \
  } while (0)

/// logging wrapper for error conditions
#define ERROR(cond)                                                            \
  ({                                                                           \
    bool cond_ = (cond);                                                       \
    if (UNLIKELY(cond_)) {                                                     \
      int errno_ = errno;                                                      \
      DEBUG("`%s` failed (current errno is %d)", #cond, errno_);               \
      errno = errno_;                                                          \
    }                                                                          \
    cond_;                                                                     \
  })
