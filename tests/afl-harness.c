#define _GNU_SOURCE
#include <assert.h>
#include <spawn.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef __APPLE__
  #include <crt_externs.h>
#endif

static char **get_environ() {
#ifdef __APPLE__
  // on macOS, environ is not directly accessible
  return *_NSGetEnviron();
#else
  return environ;
#endif
}

static const char *CFLAGS[] = {
  "-std=c11",
#ifdef __x86_64__
  "-mcx16",
#endif
  "-x",
  "c",
  "-o",
  "/dev/null",
  "-Werror=format",
  "-Werror=sign-compare",
};

static const size_t CFLAGS_size = sizeof(CFLAGS) / sizeof(CFLAGS[0]);

static const char *LDFLAGS[] = {
  "-lpthread",
};

static const size_t LDFLAGS_size = sizeof(LDFLAGS) / sizeof(LDFLAGS[0]);

static void *xcalloc(size_t count, size_t size) {
  void *p = calloc(count, size);
  if (p == NULL) {
    fprintf(stderr, "out of memory\n");
    exit(EXIT_FAILURE);
  }
  return p;
}

static char *xstrdup(const char *s) {
  char *p = strdup(s);
  if (p == NULL) {
    fprintf(stderr, "out of memory\n");
    exit(EXIT_FAILURE);
  }
  return p;
}

static char *xasprintf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  char *p = NULL;
  int r = vasprintf(&p, fmt, ap);

  va_end(ap);

  if (r == -1) {
    perror("vasprintf");
    exit(EXIT_FAILURE);
  }

  return p;
}

// run a program and return its exit status
static int run(char *const argv[]) {

  assert(argv != NULL);
  assert(argv[0] != NULL);

  // start the given process
  int r = posix_spawnp(NULL, argv[0], NULL, NULL, argv, get_environ());
  if (r != 0) {
    perror("posix_spawnp");
    exit(EXIT_FAILURE);
  }

  // wait for it to finish
  int stat_loc;
  pid_t pid = wait(&stat_loc);
  if (pid == -1) {
    perror("wait");
    exit(EXIT_FAILURE);
  }

  // if it terminated abnormally, pass an error up to AFL
  if (WIFSIGNALED(stat_loc) || WIFSTOPPED(stat_loc))
    abort();

  assert(WIFEXITED(stat_loc));

  return WEXITSTATUS(stat_loc);
}

int main(int argc, char **argv) {

  // create a temporary file for Rumur to write to
  const char *TMPDIR = getenv("TMPDIR");
  if (TMPDIR == NULL)
    TMPDIR = "/tmp";
  char *tmp = xasprintf("%s/tmp.XXXXXX", TMPDIR);
  int fd = mkstemp(tmp);
  if (fd == -1) {
    perror("mkstemp");
    return EXIT_FAILURE;
  }

  char **rumur_args = xcalloc(argc + 3, sizeof(char*));

  // setup a call to Rumur with identical args to us
  rumur_args[0] = xstrdup("rumur");
  for (size_t i = 1; i < (size_t)argc; i++)
    rumur_args[i] = argv[i];
  rumur_args[argc] = xstrdup("--output");
  rumur_args[argc + 1] = tmp;
  rumur_args[argc + 2] = NULL;

  // run Rumur
  int r = run(rumur_args);

  // if Rumur did not generate a model, we're done
  if (r != EXIT_SUCCESS)
    return r;

  // find the C compiler
  const char *cc = getenv("AFL_HARNESS_CC");
  if (cc == NULL)
    cc = getenv("CC");
  if (cc == NULL)
    cc = "cc";

  char **cc_args = xcalloc(CFLAGS_size + LDFLAGS_size + 3, sizeof(char*));

  // setup a call to the C compiler
  size_t i = 0;
  cc_args[i++] = xstrdup(cc);
  for (size_t j = 0; j < CFLAGS_size; j++)
    cc_args[i++] = xstrdup(CFLAGS[j]);
  cc_args[i++] = tmp;
  for (size_t j = 0; j < LDFLAGS_size; j++)
    cc_args[i++] = xstrdup(LDFLAGS[j]);

  // run the C compiler
  r = run(cc_args);

  // if the compiler failed to build our generated code, signal an error
  if (r != EXIT_SUCCESS)
    abort();
  
  return EXIT_SUCCESS;
}
