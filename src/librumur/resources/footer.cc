// Things that are expected to be defined above
struct State;
struct StartState;
struct Invariant;
struct Rule;

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
