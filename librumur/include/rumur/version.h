#pragma once

#ifndef RUMUR_API
#define RUMUR_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/// retrieve the version string of librumur
///
/// This version is intended to be opaque. Comparisons between multiple version
/// strings using, e.g., `strcmp` are not guaranteed to order versions. All you
/// can do with this string is print it.
///
/// \return The version of this library
RUMUR_API const char *rumur_get_version(void);

#ifdef __cplusplus
}
#endif
