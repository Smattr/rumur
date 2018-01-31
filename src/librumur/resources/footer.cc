struct state_hash {
  size_t operator()(const State *s) const {
    return s->hash();
  }
};

struct state_eq {
  bool operator()(const State *a, const State *b) const {
    return *a == *b;
  }
};

static unsigned print_counterexample(const State &s) {
  /* Recurse so that we print the states in reverse-linked order, which
   * corresponds to the order in which they were traversed.
   */
  unsigned step = 0;
  if (s.previous != nullptr) {
    step = print_counterexample(*s.previous) + 1;
  }

  print("State %u:\n", step);
  print_state(s);
  print("------------------------------------------------------------\n");
  return step;
}

static const time_t START_TIME = time(nullptr);

static unsigned long long gettime() {
  return (unsigned long long)(time(nullptr) - START_TIME);
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
      for (const Invariant &inv : INVARIANTS) {
        if (!inv.guard(*s)) {
          throw Error("invariant " + inv.name + " failed");
        }
      }
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

        // Print progress every now and then
        if (seen.size() % 1000 == 0) {
          print("%zu states seen in %llu seconds, %zu states in queue\n",
            seen.size(), gettime(), q.size());
        }

        for (const Invariant &inv : INVARIANTS) {
          if (!inv.guard(*next)) {
            throw Error("invariant " + inv.name + " failed");
          }
        }
        q.push(next);
      }

    }

    // Completed state exploration successfully.
    print("%zu states covered, no errors found\n", seen.size());

  } catch (Error e) {
    printf("%zu states covered\n", seen.size());
    fprint(stderr, "%s\n", e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
