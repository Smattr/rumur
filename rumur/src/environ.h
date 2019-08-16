#pragma once

#ifdef __APPLE__
    #include <crt_externs.h>
#endif

static inline char **get_environ() {
#ifdef __APPLE__
  // on macOS, environ is not directly accessible
  return *_NSGetEnviron();
#elif defined(__FreeBSD__)
  /* FreeBSD does not expose environ in a header, so declare it ourselves and
   * assume it will be available when linking
   */
  extern char **environ;

  return environ;
#else
  return environ;
#endif
}
