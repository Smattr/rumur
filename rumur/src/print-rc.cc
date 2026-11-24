#include "print-rc.h"
#include <cassert>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <rumur/rumur.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifdef __FreeBSD__
#include <sys/sysctl.h>
#include <sys/types.h>
#endif

/// `readlink`-alike that dynamically allocates
static char *readln(const char *path) {

  char *resolved = nullptr;
  size_t size = 512;

  while (true) {

    // expand target buffer
    size *= 2;
    {
      char *const r = reinterpret_cast<char *>(realloc(resolved, size));
      if (r == nullptr)
        break;
      resolved = r;
    }

    // attempt to resolve
    {
      const ssize_t written = readlink(path, resolved, size);
      if (written < 0)
        break;
      if ((size_t)written < size) {
        // success
        resolved[written] = '\0';
        return resolved;
      }
    }
  }

  // failed
  free(resolved);
  return nullptr;
}

/// find a path to ourselves
static char *find_me(void) {

  // Linux
  char *path = readln("/proc/self/exe");
  if (path != nullptr)
    return path;

  // DragonFly BSD, FreeBSD
  path = readln("/proc/curproc/file");
  if (path != nullptr)
    return path;

  // NetBSD
  path = readln("/proc/curproc/exe");
  if (path != nullptr)
    return path;

// /proc-less FreeBSD
#ifdef __FreeBSD__
  {
    int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
    static const size_t MIB_LENGTH = sizeof(mib) / sizeof(mib[0]);

    do {
      // determine how long the path is
      size_t buf_size = 0;
      if (sysctl(mib, MIB_LENGTH, nullptr, &buf_size, nullptr, 0) < 0)
        break;
      assert(buf_size > 0);

      // make enough space for the target path
      char *buf = reinterpret_cast<char *>(malloc(buf_size));
      if (buf == nullptr)
        break;

      // resolve it
      if (sysctl(mib, MIB_LENGTH, buf, &buf_size, nullptr, 0) == 0)
        return buf;
      free(buf);
    } while (0);
  }
#endif

  // macOS
#ifdef __APPLE__
  {
    // determine how many bytes we will need to allocate
    uint32_t buf_size = 0;
    const int rc __attribute__((unused)) =
        _NSGetExecutablePath(nullptr, &buf_size);
    assert(rc != 0);
    assert(buf_size > 0);

    path = reinterpret_cast<char *>(malloc(buf_size));
    if (path == nullptr)
      return nullptr;

    // retrieve the actual path
    if (_NSGetExecutablePath(path, &buf_size) < 0) {
      free(path);
      return nullptr;
    }

    // try to resolve any levels of symlinks if possible
    while (true) {
      char *const buf = readln(path);
      if (buf == nullptr)
        return path;

      free(path);
      path = buf;
    }
  }
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunreachable-code-return"
#endif
  return nullptr;
#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

/// is the given path a symlink?
///
/// @param path Path to check
/// @return 1 for “is a symlink”, 0 for “is not a symlink”, otherwise a negative
///   errno value
static int is_symlink(const char *path) {

  struct stat st;
  if (lstat(path, &st) < 0)
    return -errno;

  return S_ISLNK(st.st_mode) != 0;
}

/// recursively resolve symlinks
///
/// This function consumes its input, `path`. Callers should not attempt to
/// `free` or otherwise use `path` after calling this function.
///
/// @param path Path to resolve
/// @return Heap-allocated resolved path on success
static char *resolve(char *path) {
  assert(path != nullptr);

  while (true) {
    const int r = is_symlink(path);
    if (r < 0) {
      std::cerr << "failed to read attributes of " << path << ": "
                << strerror(-r) << '\n';
      free(path);
      return nullptr;
    }
    if (r == 0)
      break;
    char *const resolved = readln(path);
    if (resolved == nullptr) {
      std::cerr << "failed to read target of symlink " << path << '\n';
      free(path);
      return nullptr;
    }
    free(path);
    path = resolved;
  }
  return path;
}

int print_rc(std::ostream &out) {

  // find a canonical path to myself
  char *const a_me = find_me();
  if (a_me == nullptr) {
    std::cerr << "failed to find the path of the rumur executable\n";
    return -1;
  }
  char *const me = resolve(a_me);
  if (me == nullptr) {
    free(me);
    return -1;
  }

  // truncate this to the install prefix
  const size_t me_len = strlen(me);
  const char SUFFIX[] = "/bin/rumur";
  const size_t SUFFIX_LEN = strlen(SUFFIX);
  if (me_len < SUFFIX_LEN || strcmp(me + me_len - SUFFIX_LEN, SUFFIX) != 0) {
    std::cerr << "resolved path " << me << " does not end in " << SUFFIX
              << '\n';
    free(me);
    return -1;
  }
  me[me_len - SUFFIX_LEN] = '\0';
  const char *const root = me;

  // possible library directories in order of preference
  const char *LIB[] = {"lib", "lib64", "lib32"};

  // figure out which library directory was installed into
  const char *lib = nullptr;
  for (const char *l : LIB) {
    const std::string liba = std::string(root) + "/" + l + "/librumur.a";
    if (access(liba.c_str(), F_OK) == 0) {
      lib = l;
      break;
    }
    const std::string libso = std::string(root) + "/" + l + "/librumur.so";
    if (access(libso.c_str(), F_OK) == 0) {
      lib = l;
      break;
    }
  }
  if (lib == nullptr) {
    std::cerr << "failed to find library directory\n";
    free(me);
    return -1;
  }

  // alternate pattern:
  //   which setenv >/dev/null && test ${?FOO} -eq 0 && \
  //     setenv __UD 1 || true
  //   which setenv >/dev/null && test ${?FOO} -eq 0 && \
  //     setenv FOO "bar" || true
  //   which setenv >/dev/null && test ${?__UD} -eq 0 && \
  //     setenv FOO "bar:${FOO}" || true
  //   which setenv >/dev/null && unsetenv __UD || true

  out << "#/bin/sh\n\n"
         "# Rumur "
      << rumur_get_version()
      << " setup script\n"
         "#\n"
         "# Source this to configure Rumur in your current shell. This script\n"
         "# is intended to work in any POSIX-compatible shell, csh, or tcsh.\n"
         "\n"
         "# The idiom `which setenv >/dev/null` is used for “is this\n"
         "# csh/tcsh?” Then some gymnastics are required to set environment\n"
         "# variables on both csh/tcsh and POSIC shells without triggering\n"
         "# confusing warnings and error messages. The main complication is\n"
         "# that csh/tcsh parses full lines and examines all variables even\n"
         "# in dead branches, so all referenced variables need to be defined.\n"
         "# The basic pattern we use is:\n"
         "#\n"
         "#   which setenv >/dev/null && test ${?FOO} -eq 1 && \\\n"
         "#     setenv FLAG 1 || true\n"
         "#   which setenv >/dev/null && test ${?FOO} -eq 0 && \\\n"
         "#     setenv FLAG 0 || true\n"
         // XXX: seems like we could drop this line
         "#   which setenv >/dev/null && test ${FLAG} -eq 0 && \\\n"
         "#     setenv FOO || true\n"
         "#   which setenv >/dev/null && test ${FLAG} -eq 0 && \\\n"
         "#     setenv FOO \"bar\" || true\n"
         "#   which setenv >/dev/null && test ${FLAG} -eq 1 && \\\n"
         "#     setenv FOO \"bar:${FOO}\" || true\n"
         "#   which setenv >/dev/null && unsetenv FLAG || true\n"
         "#   which setenv >/dev/null || test \"${FOO}x\" != \"x\" && \\\n"
         "#     export FOO=\"bar:${FOO}\" || true\n"
         "#   which setenv >/dev/null || test \"${FOO}x\" = \"x\" && \\\n"
         "#     export FOO=\"bar\" || true\n";

  out << "\n"
         "# make Rumur tools runnable\n"
         "which setenv >/dev/null && setenv PATH \"${PATH}:"
      << root << "/bin\" || export PATH=\"${PATH}:" << root << "/bin\"\n";

  out << "\n"
         "# teach C++ compilers to find librumur’s headers\n"
         "which setenv >/dev/null && test ${?CPLUS_INCLUDE_PATH} -eq 1 && \\\n"
         "  setenv __HAS 1 || true\n"
         "which setenv >/dev/null && test ${?CPLUS_INCLUDE_PATH} -eq 0 && \\\n"
         "  setenv __HAS 0 || true\n"
         "which setenv >/dev/null && test ${__HAS} -eq 0 && \\\n"
         "  setenv CPLUS_INCLUDE_PATH || true\n"
         "which setenv >/dev/null && test ${__HAS} -eq 0 && \\\n"
         "  setenv CPLUS_INCLUDE_PATH \""
      << root
      << "/include\" || true\n"
         "which setenv >/dev/null && test ${__HAS} -eq 1 && \\\n"
         "  setenv CPLUS_INCLUDE_PATH \""
      << root
      << "/include:${CPLUS_INCLUDE_PATH}\" || true\n"
         "which setenv >/dev/null && unsetenv __HAS || true\n"
         "which setenv >/dev/null || test \"${CPLUS_INCLUDE_PATH}x\" != \"x\" "
         "&& \\\n"
         "  export CPLUS_INCLUDE_PATH=\""
      << root
      << "/include:${CPLUS_INCLUDE_PATH}\" || true\n"
         "which setenv >/dev/null || test \"${CPLUS_INCLUDE_PATH}x\" = \"x\" "
         "&& \\\n"
         "  export CPLUS_INCLUDE_PATH=\""
      << root << "/include\" || true\n";

  out << "\n"
         "# teach static linkers to find librumur\n"
         "which setenv >/dev/null && test ${?LIBRARY_PATH} -eq 1 && \\\n"
         "  setenv __HAS 1 || true\n"
         "which setenv >/dev/null && test ${?LIBRARY_PATH} -eq 0 && \\\n"
         "  setenv __HAS 0 || true\n"
         "which setenv >/dev/null && test ${__HAS} -eq 0 && \\\n"
         "  setenv LIBRARY_PATH || true\n"
         "which setenv >/dev/null && test ${__HAS} -eq 0 && \\\n"
         "  setenv LIBRARY_PATH \""
      << root << '/' << lib
      << "\" || true\n"
         "which setenv >/dev/null && test ${__HAS} -eq 1 && \\\n"
         "  setenv LIBRARY_PATH \""
      << root << '/' << lib
      << ":${LIBRARY_PATH}\" || true\n"
         "which setenv >/dev/null && unsetenv __HAS || true\n"
         "which setenv >/dev/null || test \"${LIBRARY_PATH}x\" != \"x\" && \\\n"
         "  export LIBRARY_PATH=\""
      << root << '/' << lib
      << ":${LIBRARY_PATH}\" || true\n"
         "which setenv >/dev/null || test \"${LIBRARY_PATH}x\" = \"x\" && \\\n"
         "  export LIBRARY_PATH=\""
      << root << '/' << lib << "\" || true\n";

  out << "\n"
         "# teach dynamic linkers to find librumur\n"
         "which setenv >/dev/null && test \"`uname`\" = \"Darwin\" -a "
         "${?DYLD_LIBRARY_PATH} -eq 1 && \\\n"
         "  setenv __HAS 1 || true\n"
         "which setenv >/dev/null && test \"`uname`\" = \"Darwin\" -a "
         "${?DYLD_LIBRARY_PATH} -eq 0 && \\\n"
         "  setenv __HAS 0 || true\n"
         "which setenv >/dev/null && test \"`uname`\" = \"Darwin\" -a ${__HAS} "
         "-eq 0 && \\\n"
         "  setenv DYLD_LIBRARY_PATH || true\n"
         "which setenv >/dev/null && test \"`uname`\" = \"Darwin\" -a ${__HAS} "
         "-eq 0 && \\\n"
         "  setenv DYLD_LIBRARY_PATH \""
      << root << '/' << lib
      << "\" || true\n"
         "which setenv >/dev/null && test \"`uname`\" = \"Darwin\" -a ${__HAS} "
         "-eq 1 && \\\n"
         "  setenv DYLD_LIBRARY_PATH \""
      << root << '/' << lib
      << ":${DYLD_LIBRARY_PATH}\" || true\n"
         "which setenv >/dev/null && unsetenv __HAS || true\n"
         "which setenv >/dev/null || test \"`uname`\" = \"Darwin\" -a "
         "\"${DYLD_LIBRARY_PATH}x\" != \"x\" && \\\n"
         "  export DYLD_LIBRARY_PATH=\""
      << root << '/' << lib
      << ":${DYLD_LIBRARY_PATH}\" || true\n"
         "which setenv >/dev/null || test \"`uname`\" = \"Darwin\" -a "
         "\"${DYLD_LIBRARY_PATH}x\" = \"x\" && \\\n"
         "  export DYLD_LIBRARY_PATH=\""
      << root << '/' << lib
      << "\" || true\n"
         "which setenv >/dev/null && test \"`uname`\" != \"Darwin\" -a "
         "${?LD_LIBRARY_PATH} -eq 1 && \\\n"
         "  setenv __HAS 1 || true\n"
         "which setenv >/dev/null && test \"`uname`\" != \"Darwin\" -a "
         "${?LD_LIBRARY_PATH} -eq 0 && \\\n"
         "  setenv __HAS 0 || true\n"
         "which setenv >/dev/null && test \"`uname`\" != \"Darwin\" -a "
         "${__HAS} -eq 0 && \\\n"
         "  setenv LD_LIBRARY_PATH || true\n"
         "which setenv >/dev/null && test \"`uname`\" != \"Darwin\" -a "
         "${__HAS} -eq 0 && \\\n"
         "  setenv LD_LIBRARY_PATH \""
      << root << '/' << lib
      << "\" || true\n"
         "which setenv >/dev/null && test \"`uname`\" != \"Darwin\" -a "
         "${__HAS} -eq 1 && \\\n"
         "  setenv LD_LIBRARY_PATH \""
      << root << '/' << lib
      << ":${LD_LIBRARY_PATH}\" || true\n"
         "which setenv >/dev/null && unsetenv __HAS || true\n"
         "which setenv >/dev/null || test \"`uname`\" != \"Darwin\" -a "
         "\"${LD_LIBRARY_PATH}x\" != \"x\" && \\\n"
         "  export LD_LIBRARY_PATH=\""
      << root << '/' << lib
      << ":${LD_LIBRARY_PATH}\" || true\n"
         "which setenv >/dev/null || test \"`uname`\" != \"Darwin\" -a "
         "\"${LD_LIBRARY_PATH}x\" = \"x\" && \\\n"
         "  export LD_LIBRARY_PATH=\""
      << root << '/' << lib << "\" || true\n";

  out << "\n"
         "# teach CMake to find librumurConfig.cmake\n"
         "which setenv >/dev/null && setenv rumur_ROOT \""
      << root << "\" || export rumur_ROOT=\"" << root << "\"\n";

  free(me);
  return 0;
}
