/* An exception that is thrown at runtime if an error is detected during model
 * checking.
 */
class ModelError : public std::runtime_error {

  public:
    using std::runtime_error::runtime_error;

};

/* Overflow-safe helpers for doing 64-bit arithmetic. The compiler built-ins
 * used are implemented in modern GCC and Clang. If you're using another
 * compiler, you'll have to implement these yourself.
 */

static int64_t add(int64_t a, int64_t b) {
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

static int64_t sub(int64_t a, int64_t b) {
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

static int64_t mul(int64_t a, int64_t b) {
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

static int64_t div(int64_t a, int64_t b) {
    if (b == 0) {
        throw ModelError("division by zero");
    }

    if constexpr (OVERFLOW_CHECKS_ENABLED) {
        if (a == INT64_MIN && b == -1) {
            throw ModelError("integer overflow in division");
        }
    }
    return a / b;
}
