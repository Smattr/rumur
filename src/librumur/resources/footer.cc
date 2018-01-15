/* Overflow-safe helpers for doing 64-bit arithmetic. The compiler built-ins
 * used are implemented in modern GCC and Clang. If you're using another
 * compiler, you'll have to implement these yourself.
 */

static int64_t add(int64_t a, int64_t b) {
    int64_t r;
    if (OVERFLOW_CHECKS_ENABLED) {
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
    if (OVERFLOW_CHECKS_ENABLED) {
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
    if (OVERFLOW_CHECKS_ENABLED) {
        if (__builtin_mul_overflow(a, b, &r)) {
            throw ModelError("integer overflow in multiplication");
        }
    } else {
        r = a * b;
    }
    return r;
}

static int64_t divide(int64_t a, int64_t b) {
    if (b == 0) {
        throw ModelError("division by zero");
    }

    if (OVERFLOW_CHECKS_ENABLED) {
        if (a == std::numeric_limits<int64_t>::min() && b == -1) {
            throw ModelError("integer overflow in division");
        }
    }
    return a / b;
}

static int64_t mod(int64_t a, int64_t b) {
    if (b == 0) {
        throw ModelError("modulus by zero");
    }

    // Is INT64_MIN % -1 UD? Reading the C spec I'm not sure.
    if (OVERFLOW_CHECKS_ENABLED) {
        if (a == std::numeric_limits<int64_t>::min() && b == -1) {
            throw ModelError("integer overflow in modulo");
        }
    }
    return a % b;
}

static int64_t negate(int64_t a) {
    if (OVERFLOW_CHECKS_ENABLED) {
        if (a == std::numeric_limits<int64_t>::min()) {
            throw ModelError("integer overflow in negation");
        }
    }
    return -a;
}

struct state_hash {
    size_t operator()(const State *s) const {
        // TODO
        return 0;
    }
};

struct state_eq {
    bool operator()(const State *a, const State *b) const {
        return *a == *b;
    }
};

static void check_invariants(const State &s) {
    for (const Invariant &inv : INVARIANTS) {
        if (!inv.guard(s))
            throw ModelError("invariant " + inv.name + " failed", &s);
    }
}

int main(void) {

    /* A queue of states to expand. A data structure invariant we maintain on
     * this collection is that all states within pass all invariants.
     */
    std::queue<State*> q;

    /* The states we have encountered. This collection will only ever grow while
     * checking the model.
     */
    std::unordered_set<State*, state_hash, state_eq> seen;

    try {

        for (const StartState &rule : START_RULES) {
            State *s = new State;
            rule.body(*s);
            // Skip this state if we've already seen it.
            if (!seen.insert(s).second) {
                delete s;
                continue;
            }
            // Check invariants eagerly.
            check_invariants(*s);
            q.push(s);
        }

        while (!q.empty()) {

            // Retrieve the next state to expand.
            State *s = q.front();
            q.pop();

            // Run each applicable rule on it, generating new states.
            for (const Rule &rule : RULES) {

                // Only consider this rule if its guard evaluates to true.
                if (!rule.guard(*s))
                    continue;

                State *next = s->duplicate();
                rule.body(*next);

                if (!seen.insert(next).second) {
                    delete next;
                    continue;
                }

                check_invariants(*next);
                q.push(next);
            }

        }

        // Completed state exploration successfully.

    } catch (ModelError e) {
        fputs(e.what(), stderr);
        print_counterexample(e.state);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
