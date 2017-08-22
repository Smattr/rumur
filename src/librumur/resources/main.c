#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static const unsigned long INVARIANTS_LEN = sizeof(INVARIANTS) / sizeof(INVARIANTS[0]);

static void check_invariants(const State *s) {
    for (unsigned long i = 0; i < INVARIANTS_LEN; i++) {
        if (!INVARIANTS[i].guard(s)) {
            char *message;
            if (asprintf(&message, "invariant %s failed", INVARIANTS[i].name) < 0) {
                perror("asprintf");
                exit(EXIT_FAILURE);
            }
            error(s, message);
            __builtin_unreachable();
        }
    }
}

static const unsigned long START_RULES_LEN = sizeof(START_RULES) / sizeof(START_RULES[0]);
static const unsigned long RULES_LEN = sizeof(RULES) / sizeof(RULES[0]);

static int main_single_threaded() {

    /* A queue of states to expand. A data structure invariant we maintain on
     * this collection is that all states within pass all invariants.
     */
    queue_t queue;
    if (queue_init(&queue) < 0) {
        perror("queue_init");
        exit(EXIT_FAILURE);
    }

    /* The states we have encountered. This collection will only ever grow while
     * checking the model.
     */
    set_t set;
    if (set_init(&set) < 0) {
        perror("set_init");
        exit(EXIT_FAILURE);
    }

    for (unsigned long i = 0; i < START_RULES_LEN; i++) {
        State *s = START_RULES[i].body();
        // Skip this state if we've already seen it.
        if (!set_insert(&set, s)) {
            free(s);
            continue;
        }
        // Check invariants eagerly.
        check_invariants(s);
        queue_push(&queue, s);
    }

    for (;;) {

        // Retrieve the next state to expand.
        State *s = queue_pop(&queue);
        if (s == NULL)
            break;

        // Run each applicable rule on it, generating new states.
        for (unsigned long i = 0; i < RULES_LEN; i++) {

            // Only consider this rule if its guard evaluates to true.
            if (!RULES[i].guard(s))
                continue;

            State *next = state_copy(s);
            assert(next != NULL);
            RULES[i].body(next);

            if (!set_insert(&set, next)) {
                free(next);
                continue;
            }

            check_invariants(next);
            queue_push(&queue, next);
        }

    }

    // Completed state exploration successfully.

    return EXIT_SUCCESS;
}

int main() {
    // TODO: In future we will support a multi-threaded algorithm
    return main_single_threaded();
}
