#include "help.h"
#include "environ.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

// The approach we take below is writing the manpage to a temporary location and
// then asking man to display it. It would be nice to avoid the temporary file
// and just pipe the manpage to man on stdin. However, man on macOS does not
// seem to support reading from pipes. Since we need a work around for at least
// macOS, we just do it uniformly through a temporary file for all platforms.

int help(const unsigned char *manpage, size_t manpage_len) {

  int rc = 0;

  // find temporary storage space
  const char *TMPDIR = getenv("TMPDIR");
  if (TMPDIR == NULL)
    TMPDIR = "/tmp";

  // create a temporary path
  char *path = NULL;
  if (asprintf(&path, "%s/temp.XXXXXX", TMPDIR) < 0)
    return ENOMEM;

  // create a file there
  int fd = mkostemp(path, O_CLOEXEC);
  if (fd < 0) {
    rc = errno;
    free(path);
    path = NULL;
    goto done;
  }

  // write the manpage to the temporary file
  for (size_t offset = 0; offset < manpage_len;) {
    const ssize_t r = write(fd, &manpage[offset], manpage_len - offset);
    if (r < 0) {
      if (errno == EINTR)
        continue;
      rc = errno;
      goto done;
    }
    assert((size_t)r <= manpage_len - offset);
    offset += (size_t)r;
  }

  // close our file handle and mark it invalid
  (void)close(fd);
  fd = -1;

  // run man to display the help text
  pid_t man = 0;
  {
    const char *argv[] = {"man",
#ifdef __linux__
                          "--local-file",
#endif
                          path, NULL};
    char *const *args = (char *const *)argv;
    if ((rc = posix_spawnp(&man, argv[0], NULL, NULL, args, get_environ())))
      goto done;
  }

  // wait for man to finish
  (void)waitpid(man, &(int){0}, 0);

  // cleanup
done:
  if (fd >= 0)
    (void)close(fd);
  if (path != NULL)
    (void)unlink(path);
  free(path);

  return rc;
}
