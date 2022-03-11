#include "help.h"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
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
  int fd = mkstemp(path.data());
  if (fd == -1) {
    ret = errno;
    std::cerr << "failed to create temporary file\n";
    goto done;
  }

  // Write the manpage to the temporary file
  {
    ssize_t r = write(fd, manpage, manpage_len);
    if (r < 0 || (size_t)r != manpage_len) {
      ret = errno;
      std::cerr << "failed to write manpage to temporary file\n";
      goto done;
    }
  }

  // Close our file handle and mark it invalid
  close(fd);
  fd = -1;

  // Run man to display the help text
  {
    std::string args("man ");
#ifdef __linux__
    args += "--local-file ";
#endif
    args += path.data();
    ret = system(args.c_str());
  }

  // Cleanup
done:
  if (fd >= 0)
    close(fd);
  if (access(path.data(), F_OK) == 0)
    (void)unlink(path.data());
  return ret;
}
