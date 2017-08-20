static int main_single_threaded() {

    std::queue<State*> q;

    for (auto [name, f] : START_RULES) {
        State *s = f();
        // TODO: invariant check here
        q.push(s);
    }

    while (!q.empty()) {
        State *s = q.front();
        q.pop();
        // TODO
    }

    return EXIT_SUCCESS;
}

int main() {
    // TODO: In future we will support a multi-threaded algorithm
    return main_single_threaded();
}
