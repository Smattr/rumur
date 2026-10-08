/// @file
/// @brief Print values of some macros for debugging purposes

#include <stdio.h>

#ifdef __APPLE__
#include <AvailabilityMacros.h>
#endif

#ifdef __linux__
#include <linux/version.h>
#endif

int main(int argc, char **argv) {

  (void)argc;
  printf("%s:\n", argv[0]);

#ifdef __aarch64__
  printf("  __aarch64__ is defined\n");
#else
  printf("  __aarch64__ is not defined\n");
#endif

#ifdef __APPLE__
  printf("  __APPLE__ is defined\n");
#else
  printf("  __APPLE__ is not defined\n");
#endif

#ifdef __clang__
  printf("  __clang__ is defined\n");
#else
  printf("  __clang__ is not defined\n");
#endif

#ifdef __FreeBSD__
  printf("  __FreeBSD__ is defined\n");
#else
  printf("  __FreeBSD__ is not defined\n");
#endif

#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16
  printf("  __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16 is defined\n");
#else
  printf("  __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16 is not defined\n");
#endif

#ifdef __GNUC__
  printf("  __GNUC__ is %lu\n", (unsigned long)__GNUC__);
#else
  printf("  __GNUC__ is not defined\n");
#endif

#ifdef __GNUC_MINOR__
  printf("  __GNUC_MINOR__ is %lu\n", (unsigned long)__GNUC_MINOR__);
#else
  printf("  __GNUC_MINOR__ is not defined\n");
#endif

#ifdef __has_feature
  printf("  __has_feature is defined\n");
#else
  printf("  __has_feature is not defined\n");
#endif

#ifdef __has_include
  printf("  __has_include is defined\n");
#else
  printf("  __has_include is not defined\n");
#endif

#ifdef __i386__
  printf("  __i386__ is defined\n");
#else
  printf("  __i386__ is not defined\n");
#endif

#ifdef __ILP32__
  printf("  __ILP32__ is defined\n");
#else
  printf("  __ILP32__ is not defined\n");
#endif

#ifdef __linux__
  printf("  __linux__ is defined\n");
#else
  printf("  __linux__ is not defined\n");
#endif

#ifdef LINUX_VERSION_CODE
  printf("  LINUX_VERSION_CODE is %lu\n", (unsigned long)LINUX_VERSION_CODE);
#else
  printf("  LINUX_VERSION_CODE is not defined\n");
#endif

#ifdef __OpenBSD__
  printf("  __OpenBSD__ is defined\n");
#else
  printf("  __OpenBSD__ is not defined\n");
#endif

#ifdef MAC_OS_X_VERSION_MAX_ALLOWED
  printf("  MAC_OS_X_VERSION_MAX_ALLOWED is %lu\n",
         (unsigned long)MAC_OS_X_VERSION_MAX_ALLOWED);
#else
  printf("  MAC_OS_X_VERSION_MAX_ALLOWED is not defined\n");
#endif

#ifdef __SIZEOF_INT128__
  printf("  __SIZEOF_INT128__ is %lu\n", (unsigned long)__SIZEOF_INT128__);
#else
  printf("  __SIZEOF_INT128__ is not defined\n");
#endif

#ifdef __SIZEOF_POINTER__
  printf("  __SIZEOF_POINTER__ is %lu\n", (unsigned long)__SIZEOF_POINTER__);
#else
  printf("  __SIZEOF_POINTER__ is not defined\n");
#endif

#ifdef __SSE2__
  printf("  __SSE2__ is defined\n");
#else
  printf("  __SSE2__ is not defined\n");
#endif

#ifdef __x86_64__
  printf("  __x86_64__ is defined\n");
#else
  printf("  __x86_64__ is not defined\n");
#endif

  return 0;
}
