#pragma once

#include <stdio.h>

/// reformat a Murphi stream
///
/// \param dst Output stream to write to
/// \param src Input stream to read from
/// \return 0 on success or an errno on failure
int format(FILE *dst, FILE *src);
