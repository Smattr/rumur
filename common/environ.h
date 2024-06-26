#pragma once

#include "environ.h"
#include <cstdlib>

#ifdef __APPLE__
#include <crt_externs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

static inline char **get_environ(void) {
#ifdef __APPLE__
  // on macOS, environ is not directly accessible
  return *_NSGetEnviron();
#else
  /* some platforms fail to expose environ in a header (e.g. FreeBSD), so
   * declare it ourselves and assume it will be available when linking
   */
  extern char **environ;

  return environ;
#endif
}

#ifdef __cplusplus
}
#endif
