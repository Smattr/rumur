static void print_counterexample(const State *last) {
    if (last != NULL) {
        print_counterexample(last->previous);
        // TODO print state
    }
}

// Function that is called if an error is detected during model checking.
static _Noreturn void error(const State *s, const char *message) {
    // TODO
    exit(EXIT_FAILURE);
}

/* Overflow-safe helpers for doing 64-bit arithmetic. The compiler built-ins
 * used are implemented in modern GCC and Clang. If you're using another
 * compiler, you'll have to implement these yourself.
 */

static int64_t __attribute__((unused)) add(const State *s, int64_t a, int64_t b) {
    int64_t r;
    if (OVERFLOW_CHECKS_ENABLED) {
        if (__builtin_add_overflow(a, b, &r)) {\
            error(s, "integer overflow in addition");
        }
    } else {
        r = a + b;
    }
    return r;
}

static int64_t __attribute__((unused)) sub(const State *s, int64_t a, int64_t b) {
    int64_t r;
    if (OVERFLOW_CHECKS_ENABLED) {
        if (__builtin_sub_overflow(a, b, &r)) {
            error(s, "integer overflow in subtraction");
        }
    } else {
        r = a - b;
    }
    return r;
}

static int64_t __attribute__((unused)) mul(const State *s, int64_t a, int64_t b) {
    int64_t r;
    if (OVERFLOW_CHECKS_ENABLED) {
        if (__builtin_mul_overflow(a, b, &r)) {
            error(s, "integer overflow in multiplication");
        }
    } else {
        r = a * b;
    }
    return r;
}

static int64_t __attribute__((unused)) divide(const State *s, int64_t a, int64_t b) {
    if (b == 0) {
        error(s, "division by zero");
    }

    if (OVERFLOW_CHECKS_ENABLED) {
        if (a == INT64_MIN && b == -1) {
            error(s, "integer overflow in division");
        }
    }
    return a / b;
}
