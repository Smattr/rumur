#pragma once

#include <cassert>

#define BUG_STUB() \
    do { \
        assert(!"unreachable"); \
        __builtin_unreachable(); \
    } while (0)
