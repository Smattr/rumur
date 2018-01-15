#include <bitset>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <limits>
#include <queue>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <unordered_set>
#include <utility>

template<size_t SIZE_BITS>
struct StateBase {
    std::bitset<SIZE_BITS> data;
    const StateBase *previous = nullptr;

  public:
    StateBase() = default;
    StateBase(const StateBase *s): data(s->data), previous(s) {
    }

    bool operator==(const StateBase &other) const {
        return data == other.data;
    }

    bool operator!=(const StateBase &other) const {
        return !(*this == other);
    }
};

template<typename STATE>
struct StartStateBase {
    std::string name;
    std::function<void(STATE&)> body;
};

template<typename STATE>
struct InvariantBase {
    std::string name;
    std::function<bool(const STATE&)> guard;
};

template<typename STATE>
struct RuleBase {
    std::string name;
    std::function<bool(const STATE&)> guard;
    std::function<void(STATE&)> body;
};

/* An exception that is thrown at runtime if an error is detected during model
 * checking.
 */
template<typename STATE>
class ModelError : public std::runtime_error {

  public:
    const STATE *state;

    ModelError(const std::string &message, const STATE *state_ = nullptr):
      std::runtime_error(message), state(state_) {
    }

    ModelError(const ModelError &e, const STATE *state_):
      std::runtime_error(e.what()), state(state_) {
    }

};

/* Overflow-safe helpers for doing 64-bit arithmetic. The compiler built-ins
 * used are implemented in modern GCC and Clang. If you're using another
 * compiler, you'll have to implement these yourself.
 */

[[maybe_unused]] static int64_t add(int64_t a, int64_t b) {
    int64_t r;
    if constexpr (OVERFLOW_CHECKS_ENABLED) {
        if (__builtin_add_overflow(a, b, &r)) {
            throw ModelError("integer overflow in addition");
        }
    } else {
        r = a + b;
    }
    return r;
}

[[maybe_unused]] static int64_t sub(int64_t a, int64_t b) {
    int64_t r;
    if constexpr (OVERFLOW_CHECKS_ENABLED) {
        if (__builtin_sub_overflow(a, b, &r)) {
            throw ModelError("integer overflow in subtraction");
        }
    } else {
        r = a - b;
    }
    return r;
}

[[maybe_unused]] static int64_t mul(const State *s, int64_t a, int64_t b) {
    int64_t r;
    if constexpr (OVERFLOW_CHECKS_ENABLED) {
        if (__builtin_mul_overflow(a, b, &r)) {
            throw ModelError("integer overflow in multiplication");
        }
    } else {
        r = a * b;
    }
    return r;
}

[[maybe_unused]] static int64_t divide(const State *s, int64_t a, int64_t b) {
    if (b == 0) {
        throw ModelError("division by zero");
    }

    if constexpr (OVERFLOW_CHECKS_ENABLED) {
        if (a == std::numeric_limits<int64_t>::min() && b == -1) {
            throw ModelError("integer overflow in division");
        }
    }
    return a / b;
}

[[maybe_unused]] static int64_t mod(const State *s, int64_t a, int64_t b) {
    if (b == 0) {
        throw ModelError("modulus by zero");
    }

    // Is INT64_MIN % -1 UD? Reading the C spec I'm not sure.
    if constexpr (OVERFLOW_CHECKS_ENABLED) {
        if (a == std::numeric_limits<int64_t>::min() && b == -1) {
            throw ModelError("integer overflow in modulo");
        }
    }
    return a % b;
}

[[maybe_unused]] static int64_t negate(const State *s, int64_t a) {
    if constexpr (OVERFLOW_CHECKS_ENABLED) {
        if (a == std::numeric_limits<int64_t>::min()) {
            throw ModelError("integer overflow in negation");
        }
    }
    return -a;
}
