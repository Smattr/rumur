static int main_single_threaded() {

    std::queue<State*> q;

    try {

        for (auto [name, f] : START_RULES) {
            State *s = f();
            // Check invariants eagerly.
            for (auto [inv_name, i] : INVARIANTS) {
                if (!i(*s))
                    throw ModelError("invariant " + inv_name + " failed");
            }
            q.push(s);
        }

        while (!q.empty()) {
            State *s = q.front();
            q.pop();
            // TODO
        }

    } catch (ModelError &e) {
        fputs(e.what(), stderr);;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main() {
    // TODO: In future we will support a multi-threaded algorithm
    return main_single_threaded();
}
