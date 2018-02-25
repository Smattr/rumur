namespace {
struct state_hash {
  size_t operator()(const State *s) const {
    return s->hash();
  }
};
}

namespace {
struct state_eq {
  bool operator()(const State *a, const State *b) const {
    return *a == *b;
  }
};
}

static unsigned print_counterexample(const State &s) {
  /* Recurse so that we print the states in reverse-linked order, which
   * corresponds to the order in which they were traversed.
   */
  unsigned step = 0;
  if (s.previous != State::ORIGIN) {
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

namespace {
using StateQueue = Queue<State, THREADS>;
}
namespace {
using StateSet = Set<State, state_hash, state_eq, SET_CAPACITY, THREADS>;
}

/* Thread data. */
static Semaphore barrier;
static std::vector<std::thread> threads;
static std::atomic_bool done;
static int exit_code;
static std::array<Allocator<State>, THREADS> allocator;

/* The states we have encountered. This collection will only ever grow while
 * checking the model.
 */
static StateSet seen;

/* A queue of states to expand. A data structure invariant we maintain on
 * this collection is that all states within pass all invariants.
 */
static StateQueue q;

static void explore(unsigned long thread_id) {

  /* In a multithreaded checker, there is one primary thread and at least one
   * secondary thread. The secondary threads initially block and then wait for
   * the primary thread to exit the "warm up" phase and notify them that they
   * should begin exploration.
   */
  const bool primary = THREADS > 1 && thread_id == 0;
  const bool secondary = THREADS > 1 && thread_id > 0;

  /* Phase of state exploration. This is only relevant in multithreaded mode and
   * then only relevant to the primary thread.
   */
  enum {
    WARM_UP,
    GO,
  } phase = WARM_UP;

  if (secondary) {
    barrier.wait();
  }

  size_t queued = q.size();
  unsigned long last_queue_id = thread_id;

  for (;;) {

    if (done.load()) {
      return;
    }

    // Retrieve the next state to expand.
    State *s = q.pop(last_queue_id);
    if (s == nullptr) {
      break;
    }
    ASSERT(queued > 0);
    queued--;

    // Run each applicable rule on it, generating new states.
    for (const Rule &rule : RULES) {
      try {
        for (State *next : rule.get_iterable(*s, allocator[thread_id])) {

          std::tuple<size_t, bool, State*> seen_result = seen.insert(next);
          size_t seen_size = std::get<0>(seen_result);
          bool seen_inserted = std::get<1>(seen_result);
          if (!seen_inserted) {
            allocator[thread_id].free(next);
            continue;
          }

          /* If the set insertion juggled our pointers, make sure we reference
           * the set-contained one from this point forward.
           */
          if (std::get<2>(seen_result) != next) {
            allocator[thread_id].free(next);
            next = std::get<2>(seen_result);
          }

          // Queue the state for expansion in future
          size_t q_size;
          if (primary && phase == WARM_UP) {
            q_size = q.push(next, last_queue_id);
            last_queue_id = (last_queue_id + 1) % THREADS;
          } else {
            q_size = q.push(next, thread_id);
            last_queue_id = thread_id;
          }
          queued++;

          if (primary && phase == WARM_UP && queued >= THREADS * 2) {
            last_queue_id = 0;
            barrier.post(THREADS - 1);
            phase = GO;
          }

          // Print progress every now and then
          if (seen_size % 10000 == 0) {
            if (THREADS > 1) {
              print("thread %lu: %zu states seen in %llu seconds, %zu states "
                "in local queue\n", thread_id, seen_size, gettime(),
                q_size);
            } else {
              print("%zu states seen in %llu seconds, %zu states in queue\n",
                seen_size, gettime(), q_size);
            }
          }

          for (const Invariant &inv : INVARIANTS) {
            if (!inv.guard(*next)) {
              s = next;
              throw Error("invariant " + inv.name + " failed");
            }
          }
        }
      } catch (Error e) {

        /* Flag that we've found an error and are exiting. */
        bool d = false;
        if (!done.compare_exchange_strong(d, true)) {
          /* Someone else beat us to it and already found an error. Exit and let
           * only their's be reported.
           */
          return;
        }

        print_counterexample(*s);
        fprint(stderr, "rule %s caused: %s\n", rule.name.c_str(), e.what());
        exit_code = EXIT_FAILURE;
        return;
      }
    }
  }

  // Completed state exploration successfully.
}

int main(void) {

  print("State size: %zu bits\n", State::width());

  for (const StartState &rule : START_RULES) {
    State *s = new State;
    s->previous = State::ORIGIN;
    try {
      rule.body(*s);
    } catch (Error e) {
      fprint(stderr, "in start state %s: %s\n", rule.name.c_str(), e.what());
      return EXIT_FAILURE;
    }
    std::tuple<size_t, bool, State*> seen_result = seen.insert(s);
    bool seen_inserted = std::get<1>(seen_result);
    // Skip this state if we've already seen it.
    if (!seen_inserted) {
      delete s;
      continue;
    }

    /* If the insertion juggled our pointers, ensure we reference the
     * set-contained one from this point on.
     */
    if (std::get<2>(seen_result) != s) {
      delete s;
      s = std::get<2>(seen_result);
    }

    // Check invariants eagerly.
    for (const Invariant &inv : INVARIANTS) {
      if (!inv.guard(*s)) {
        fprint(stderr, "start state %s failed invariant %s\n",
          rule.name.c_str(), inv.name.c_str());
        return EXIT_FAILURE;
      }
    }
    q.push(s, 0);
  }

  done = false;
  exit_code = EXIT_SUCCESS;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtautological-compare"
  for (unsigned long i = 0; i < THREADS - 1; i++) {
#pragma GCC diagnostic pop
    threads.emplace_back(explore, i + 1);
  }

  explore(0);

  /* Pump the thread barrier in case we never actually woke up the secondary
   * threads.
   */
  barrier.post(THREADS - 1);

  for (std::thread &t : threads) {
    t.join();
  }

  print("%zu states covered%s\n", seen.size(),
    exit_code == EXIT_SUCCESS ? ", no errors found" : "");

  return exit_code;
}
