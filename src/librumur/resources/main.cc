static void check_invariants(const State *s) {
    for (auto [name, f] : INVARIANTS) {
        if (!f(*s))
            throw ModelError("invariant " + name + " failed", s);
    }
}

static int main_single_threaded() {

    /* A queue of states to expand. A data structure invariant we maintain on
     * this collection is that all states within pass all invariants.
     */
    std::queue<State*> q;

    /* The states we have encountered. This collection will only ever grow while
     * checking the model.
     */
    std::unordered_set<State*, state_hash> seen;

    try {

        for (auto [name, f] : START_RULES) {
            State *s = f();
            // Skip this state if we've already seen it.
            if (!seen.insert(s).second) {
                delete s;
                continue;
            }
            // Check invariants eagerly.
            check_invariants(s);
            q.push(s);
        }

        while (!q.empty()) {

            // Retrieve the next state to expand.
            State *s = q.front();
            q.pop();

            // Run each applicable rule on it, generating new states.
            for (auto [name, g, f] : RULES) {

                // Only consider this rule if its guard evaluates to true.
                if (!g(*s))
                    continue;

                State *next = s->duplicate();
                f(next);

                if (!seen.insert(next).second) {
                    delete next;
                    continue;
                }

                check_invariants(next);
                q.push(next);
            }

        }

        // Completed state exploration successfully.

    } catch (ModelError &e) {
        fputs(e.what(), stderr);
        print_counterexample(e.state);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main() {
    // TODO: In future we will support a multi-threaded algorithm
    return main_single_threaded();
}
