#include "help.h"
#include "environ.h"
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <spawn.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

/* The approach we take below is writing the manpage to a temporary location and
 * then asking man to display it. It would be nice to avoid the temporary file
 * and just pipe the manpage to man on stdin. However, man on macOS does not
 * seem to support reading from pipes. Since we need a work around for at least
 * macOS, we just do it uniformly through a temporary file for all platforms.
 */

int help(const unsigned char *manpage, size_t manpage_len) {

  int ret = 0;

  // Find temporary storage space
  std::string tmp;
  const char *TMPDIR = getenv("TMPDIR");
  if (TMPDIR == nullptr) {
    tmp = "/tmp";
  } else {
    tmp = TMPDIR;
  }

  // Create a temporary file
  size_t size = tmp.size() + sizeof("/temp.XXXXXX");
  std::vector<char> path(size);
  snprintf(path.data(), path.size(), "%s/temp.XXXXXX", tmp.c_str());
  int fd = mkostemp(path.data(), O_CLOEXEC);
  if (fd == -1) {
    ret = errno;
    path.clear();
    std::cerr << "failed to create temporary file\n";
    goto done;
  }

  // Write the manpage to the temporary file
  for (size_t offset = 0; offset < manpage_len;) {
    const ssize_t r = write(fd, &manpage[offset], manpage_len - offset);
    if (r < 0) {
      if (errno == EINTR)
        continue;
      ret = errno;
      goto done;
    }
    assert((size_t)r <= manpage_len - offset);
    offset += (size_t)r;
  }

  // Close our file handle and mark it invalid
  close(fd);
  fd = -1;

  // Run man to display the help text
  pid_t man;
  {
    const char *argv[] = {"man",
#ifdef __linux__
                          "--local-file",
#endif
                          path.data(), nullptr};
    char *const *args = const_cast<char *const *>(argv);
    if ((ret = posix_spawnp(&man, argv[0], nullptr, nullptr, args,
                            get_environ())))
      goto done;
  }

  // wait for man to finish
  int ignored;
  (void)waitpid(man, &ignored, 0);

  // Cleanup
done:
  if (fd >= 0)
    close(fd);
  if (!path.empty())
    (void)unlink(path.data());
  return ret;
}
