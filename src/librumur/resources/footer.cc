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

  fprint(stderr, "State %u:\n", step);
  print_state(s);
  fprint(stderr, "------------------------------------------------------------\n");
  return step;
}

static const time_t START_TIME = time(nullptr);

static unsigned long long gettime() {
  return (unsigned long long)(time(nullptr) - START_TIME);
}

using StateQueue = Queue<State, THREADS>;
using StateSet = Set<State, state_hash, state_eq, THREADS>;

static int explore(StateQueue &q, StateSet &seen) {
  for (;;) {

    // Retrieve the next state to expand.
    State *s = q.pop();
    if (s == nullptr) {
      break;
    }

    // Run each applicable rule on it, generating new states.
    for (const Rule &rule : RULES) {
      try {
        for (State *next : rule.get_iterable(*s)) {

          std::pair<size_t, bool> seen_result = seen.insert(next);
          if (!seen_result.second) {
            delete next;
            continue;
          }

          // Queue the state for expansion in future
          size_t q_size = q.push(next);

          // Print progress every now and then
          if (seen_result.first % 10000 == 0) {
            print("%zu states seen in %llu seconds, %zu states in queue\n",
              seen_result.first, gettime(), q_size);
          }

          for (const Invariant &inv : INVARIANTS) {
            if (!inv.guard(*next)) {
              s = next;
              throw Error("invariant " + inv.name + " failed");
            }
          }
        }
      } catch (Error e) {
        print_counterexample(*s);
        fprint(stderr, "rule %s caused: %s\n", rule.name.c_str(), e.what());
        return EXIT_FAILURE;
      }
    }
  }

  // Completed state exploration successfully.
  return EXIT_SUCCESS;
}

int main(void) {

  print("State size: %zu bits\n", State::width());

  /* A queue of states to expand. A data structure invariant we maintain on
   * this collection is that all states within pass all invariants.
   */
  StateQueue q;

  /* The states we have encountered. This collection will only ever grow while
   * checking the model.
   */
  StateSet seen;

  for (const StartState &rule : START_RULES) {
    State *s = new State;
    try {
      rule.body(*s);
    } catch (Error e) {
      fprint(stderr, "in start state %s: %s\n", rule.name.c_str(), e.what());
      return EXIT_FAILURE;
    }
    // Skip this state if we've already seen it.
    if (!seen.insert(s).second) {
      delete s;
      continue;
    }
    // Check invariants eagerly.
    for (const Invariant &inv : INVARIANTS) {
      if (!inv.guard(*s)) {
        fprint(stderr, "start state %s failed invariant %s\n",
          rule.name.c_str(), inv.name.c_str());
        return EXIT_FAILURE;
      }
    }
    q.push(s);
  }

  int ret = explore(q, seen);

  print("%zu states covered%s\n", seen.size(),
    ret == EXIT_SUCCESS ? ", no errors found" : "");

  return ret;
}
