#ifndef __OPTIMIZE__
  #ifdef __clang__
    #ifdef __x86_64__
      #warning you are compiling without optimizations enabled. I would suggest -march=native -O3 -mcx16.
    #else
      #warning you are compiling without optimizations enabled. I would suggest -march=native -O3.
    #endif
  #else
    #ifdef __x86_64__
      #warning you are compiling without optimizations enabled. I would suggest -march=native -O3 -fwhole-program -mcx16.
    #else
      #warning you are compiling without optimizations enabled. I would suggest -march=native -O3 -fwhole-program.
    #endif
  #endif
#endif

#define value_to_string(v) ((value_t)(v))
#define raw_value_to_string(v) ((raw_value_t)(v))

/* A more powerful assert that treats the assertion as an assumption when
 * assertions are disabled.
 */
#ifndef NDEBUG
  #define ASSERT(expr) assert(expr)
#elif defined(__clang__)
  #define ASSERT(expr) __builtin_assume(expr)
#else
  /* GCC doesn't have __builtin_assume, so we need something else. */
  #define ASSERT(expr) \
    do { \
      /* The following is an idiom for teaching the compiler an assumption. */ \
      if (!(expr)) { \
        __builtin_unreachable(); \
      } \
    } while (0)
#endif

#define BITS_TO_BYTES(size) (size / 8 + (size % 8 == 0 ? 0 : 1))

/* The size of the compressed state data in bytes. */
enum { STATE_SIZE_BYTES = BITS_TO_BYTES(STATE_SIZE_BITS) };

/* Implement _Thread_local for GCC <4.9, which is missing this. */
#if defined(__GNUC__) && defined(__GNUC_MINOR__)
  #if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 9)
    #define _Thread_local __thread
  #endif
#endif

#ifdef __clang__
  #define NONNULL _Nonnull
#else
  #define NONNULL /* nothing; other compilers don't have _Nonnull */
#endif

/* A word about atomics... There are two different atomic operation mechanisms
 * used in this code and it may not immediately be obvious why one was not
 * sufficient. The two are:
 *
 *   1. GCC __atomic built-ins: Used for variables that are sometimes accessed
 *      with atomic semantics and sometimes as regular memory operations. The
 *      C11 atomics cannot give us this and the __atomic built-ins are
 *      implemented by the major compilers.
 *   2. GCC __sync built-ins: used for 128-bit atomic accesses on x86-64. It
 *      seems the __atomic built-ins do not result in a CMPXCHG instruction, but
 *      rather in a less efficient library call. See
 *      https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80878.
 *
 * Though this is intended to be a C11 program, we avoid the C11 atomics to be
 * compatible with GCC <4.9.
 */

/* Identifier of the current thread. This counts up from 0 and thus is suitable
 * to use for, e.g., indexing into arrays. The initial thread has ID 0.
 */
static _Thread_local size_t thread_id;

/* The threads themselves. Note that we have no element for the initial thread,
 * so *your* thread is 'threads[thread_id - 1]'.
 */
static pthread_t threads[THREADS - 1];

/* What we are currently doing. Either "warming up" (running single threaded
 * building up queue occupancy) or "free running" (running multithreaded).
 */
static enum { WARMUP, RUN } phase = WARMUP;

/* Number of errors we've noted so far. If a thread sees this hit or exceed
 * MAX_ERRORS, they should attempt to exit gracefully as soon as possible.
 */
static unsigned long error_count;

/* Number of rules that have been processed. There are two representations of
 * this: a thread-local count of how many rules we have fired thus far and a
 * global array of *final* counts of fired rules per-thread that is updated and
 * used as threads are exiting. The purpose of this duplication is to let the
 * compiler layout the thread-local variable in a cache-friendly way and use
 * this during checking, rather than having all threads contending on the global
 * array whose entries are likely all within the same cache line.
 */
static _Thread_local uintmax_t rules_fired_local;
static uintmax_t rules_fired[THREADS];

/* Checkpoint to restore to after reporting an error. This is only used if we
 * are tolerating more than one error before exiting.
 */
static _Thread_local sigjmp_buf checkpoint;

_Static_assert(MAX_ERRORS > 0, "illegal MAX_ERRORS value");

/* Whether we need to save and restore checkpoints. This is determined by
 * whether we ever need to perform the action "discard the current state and
 * skip to checking the next." This scenario can only occur for one reason: We
 * have just found an error and have not yet hit MAX_ERRORS. In this case we
 * want to longjmp back to resume checking.
 */
enum { JMP_BUF_NEEDED = MAX_ERRORS > 1 };

/*******************************************************************************
 * Sandbox support.                                                            *
 *                                                                             *
 * Because we're running generated code, it seems wise to use OS mechanisms to *
 * reduce our privileges, where possible.                                      *
 ******************************************************************************/

static void sandbox(void) {

  if (!SANDBOX_ENABLED) {
    return;
  }

#ifdef __APPLE__
  {
    char *err;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    int r = sandbox_init(kSBXProfilePureComputation, SANDBOX_NAMED, &err);
#pragma clang diagnostic pop

    if (r != 0) {
      fprintf(stderr, "sandbox_init failed: %s\n", err);
      free(err);
      exit(EXIT_FAILURE);
    }

    return;
  }
#endif

#ifdef __FreeBSD__
  {
    if (cap_enter() != 0) {
      perror("cap_enter");
      exit(EXIT_FAILURE);
    }
    return;
  }
#endif

#if defined(__linux__)
  #if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0)
  {
    /* Disable the addition of new privileges via execve and friends. */
    int r = prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);
    if (r != 0) {
      perror("prctl(PR_SET_NO_NEW_PRIVS) failed");
      exit(EXIT_FAILURE);
    }

    /* A BPF program that traps on any syscall we want to disallow. */
    static struct sock_filter filter[] = {

#if 0
      // TODO: The following will require some pesky ifdef mess because the
      // Linux headers don't seem to define a "current architecture" constant.
      /* Validate that we're running on the same architecture we were compiled
       * for. If not, the syscall numbers we're using may be wrong.
       */
      BPF_STMT(BPF_LD|BPF_W|BPF_ABS, offsetof(struct seccomp_data, arch)),
      BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, ARCH_NR, 1, 0),
      BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_TRAP),
#endif

      /* Load syscall number. */
      BPF_STMT(BPF_LD|BPF_W|BPF_ABS, offsetof(struct seccomp_data, nr)),

      /* Enable exiting. */
      BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, __NR_exit_group, 0, 1),
      BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_ALLOW),

      /* Enable syscalls used by printf. */
      BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, __NR_fstat, 0, 1),
      BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_ALLOW),
      BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, __NR_write, 0, 1),
      BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_ALLOW),

      /* Enable syscalls used by malloc. */
      BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, __NR_brk, 0, 1),
      BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_ALLOW),
      BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, __NR_mmap, 0, 1),
      BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_ALLOW),
      BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, __NR_munmap, 0, 1),
      BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_ALLOW),

      /* If we're running multithreaded, enable syscalls that used by pthreads.
       */
      BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, __NR_clone, 0, 1),
      BPF_STMT(BPF_RET|BPF_K, THREADS > 1 ? SECCOMP_RET_ALLOW : SECCOMP_RET_TRAP),
      BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, __NR_close, 0, 1),
      BPF_STMT(BPF_RET|BPF_K, THREADS > 1 ? SECCOMP_RET_ALLOW : SECCOMP_RET_TRAP),
      BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, __NR_exit, 0, 1),
      BPF_STMT(BPF_RET|BPF_K, THREADS > 1 ? SECCOMP_RET_ALLOW : SECCOMP_RET_TRAP),
      BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, __NR_futex, 0, 1),
      BPF_STMT(BPF_RET|BPF_K, THREADS > 1 ? SECCOMP_RET_ALLOW : SECCOMP_RET_TRAP),
      BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, __NR_get_robust_list, 0, 1),
      BPF_STMT(BPF_RET|BPF_K, THREADS > 1 ? SECCOMP_RET_ALLOW : SECCOMP_RET_TRAP),
      BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, __NR_madvise, 0, 1),
      BPF_STMT(BPF_RET|BPF_K, THREADS > 1 ? SECCOMP_RET_ALLOW : SECCOMP_RET_TRAP),
      BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, __NR_mprotect, 0, 1),
      BPF_STMT(BPF_RET|BPF_K, THREADS > 1 ? SECCOMP_RET_ALLOW : SECCOMP_RET_TRAP),
      // XXX: it would be nice to avoid open() but pthreads seems to open libgcc.
      BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, __NR_open, 0, 1),
      BPF_STMT(BPF_RET|BPF_K, THREADS > 1 ? SECCOMP_RET_ALLOW : SECCOMP_RET_TRAP),
      BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, __NR_read, 0, 1),
      BPF_STMT(BPF_RET|BPF_K, THREADS > 1 ? SECCOMP_RET_ALLOW : SECCOMP_RET_TRAP),
      BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, __NR_set_robust_list, 0, 1),
      BPF_STMT(BPF_RET|BPF_K, THREADS > 1 ? SECCOMP_RET_ALLOW : SECCOMP_RET_TRAP),

      /* Deny everything else. On a disallowed syscall, we trap instead of
       * killing to allow the user to debug the failure. If you are debugging
       * seccomp denials, strace the checker and find the number of the denied
       * syscall in the first si_value parameter reported in the terminating
       * SIG_SYS.
       */
      BPF_STMT(BPF_RET|BPF_K, SECCOMP_RET_TRAP),
    };

    static const struct sock_fprog filter_program = {
      .len = sizeof(filter) / sizeof(filter[0]),
      .filter = filter,
    };

    /* Apply the above filter to ourselves. */
    r = prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &filter_program, 0, 0);
    if (r != 0) {
      perror("prctl(PR_SET_SECCOMP) failed");
      exit(EXIT_FAILURE);
    }

    return;
  }
  #endif
#endif

#ifdef __OpenBSD__
  {
    if (pledge("stdio", "") != 0) {
      perror("pledge");
      exit(EXIT_FAILURE);
    }
    return;
  }
#endif

  /* No sandbox available. */
  fprintf(stderr, "no sandboxing facilities available\n");
  exit(EXIT_FAILURE);
}

/******************************************************************************/

// ANSI colour code support.

static bool istty;

static const char *green() {
  if (COLOR == ON || (COLOR == AUTO && istty))
    return "\033[32m";
  return "";
}

static const char *red() {
  if (COLOR == ON || (COLOR == AUTO && istty))
    return "\033[31m";
  return "";
}

static const char *yellow() {
  if (COLOR == ON || (COLOR == AUTO && istty))
    return "\033[33m";
  return "";
}

static const char *bold() {
  if (COLOR == ON || (COLOR == AUTO && istty))
    return "\033[1m";
  return "";
}

static const char *reset() {
  if (COLOR == ON || (COLOR == AUTO && istty))
    return "\033[0m";
  return "";
}

/*******************************************************************************
 * MurmurHash by Austin Appleby                                                *
 *                                                                             *
 * More information on this at https://github.com/aappleby/smhasher/           *
 ******************************************************************************/

static uint64_t MurmurHash64A(const void *NONNULL key, size_t len) {

  static const uint64_t seed = 0;

  static const uint64_t m = UINT64_C(0xc6a4a7935bd1e995);
  static const unsigned r = 47;

  uint64_t h = seed ^ (len * m);

  const unsigned char *data = key;
  const unsigned char *end = data + len / sizeof(uint64_t) * sizeof(uint64_t);

  while (data != end) {

    uint64_t k;
    memcpy(&k, data, sizeof(k));
    data += sizeof(k);

    k *= m;
    k ^= k >> r;
    k *= m;

    h ^= k;
    h *= m;
  }

  const unsigned char *data2 = data;

  switch (len & 7) {
    case 7: h ^= (uint64_t)data2[6] << 48; /* fall through */
    case 6: h ^= (uint64_t)data2[5] << 40; /* fall through */
    case 5: h ^= (uint64_t)data2[4] << 32; /* fall through */
    case 4: h ^= (uint64_t)data2[3] << 24; /* fall through */
    case 3: h ^= (uint64_t)data2[2] << 16; /* fall through */
    case 2: h ^= (uint64_t)data2[1] << 8; /* fall through */
    case 1: h ^= (uint64_t)data2[0];
    h *= m;
  }

  h ^= h >> r;
  h *= m;
  h ^= h >> r;

  return h;
}

/******************************************************************************/

/* Signal an out-of-memory condition and terminate abruptly. */
static _Noreturn void oom(void) {
  fputs("out of memory", stderr);
  exit(EXIT_FAILURE);
}

static void *xmalloc(size_t size) {
  void *p = malloc(size);
  if (p == NULL) {
    oom();
  }
  return p;
}

static void *xcalloc(size_t count, size_t size) {
  void *p = calloc(count, size);
  if (p == NULL) {
    oom();
  }
  return p;
}

/* A lock that should be held whenever printing to stdout or stderr. This is a
 * way to prevent the output of one thread being interleaved with the output of
 * another.
 */
static pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

static void print_lock(void) {
  int r __attribute__((unused)) = pthread_mutex_lock(&print_mutex);
  assert(r == 0);
}

static void print_unlock(void) {
  int r __attribute__((unused)) = pthread_mutex_unlock(&print_mutex);
  assert(r == 0);
}

static void xml_printf(const char *NONNULL s) {
  while (*s != '\0') {
    switch (*s) {
      case '"': printf("&quot;"); break;
      case '<': printf("&lt;");   break;
      case '>': printf("&gt;");   break;
      case '&': printf("&amp;");  break;
      default:  printf("%c", *s); break;
    }
    s++;
  }
}

/* Supporting for tracing specific operations. This can be enabled during
 * checker generation with '--trace ...' and is useful for debugging Rumur
 * itself.
 */
static __attribute__((format(printf, 1, 2))) void trace(const char *NONNULL fmt,
    ...) {

  va_list ap;
  va_start(ap, fmt);

  print_lock();

  (void)fprintf(stderr, "%sTRACE%s:", yellow(), reset());
  (void)vfprintf(stderr, fmt, ap);
  (void)fprintf(stderr, "\n");

  print_unlock();
  va_end(ap);
}

/* Wrap up trace() as a macro. It looks as if the following could just be
 * incorporated into trace(). However, present compilers seem unwilling to
 * inline varargs functions or do interprocedural analysis across a call to one.
 * As a result, the compiler does not notice when tracing is disabled and a call
 * to trace() would be a no-op that can be elided. By making the call a macro we
 * make the category comparison visible to the compiler's optimising passes.
 */
#define TRACE(category, args...)                                               \
  do {                                                                         \
    if ((category) & TRACES_ENABLED) {                                         \
      trace(args);                                                             \
    }                                                                          \
  } while (0)

/*******************************************************************************
 * Arithmetic wrappers                                                         *
 *                                                                             *
 * For compilers that support them, we call the overflow built-ins to check    *
 * undefined operations during arithmetic. For others, we just emit the bare   *
 * operation.                                                                  *
 ******************************************************************************/

#if defined(__clang__) || (defined(__GNUC__) && __GNUC__ >= 5)

  #define ADD(a, b, c) __builtin_add_overflow((a), (b), (c))
  #define MUL(a, b, c) __builtin_mul_overflow((a), (b), (c))
  #define SUB(a, b, c) __builtin_sub_overflow((a), (b), (c))

#else

  #define ADD(a, b, c) ({ *(c) = (a) + (b); false; })
  #define MUL(a, b, c) ({ *(c) = (a) * (b); false; })
  #define SUB(a, b, c) ({ *(c) = (a) - (b); false; })

#endif

/******************************************************************************/

/* The state of the current model. */
struct state {
#if COUNTEREXAMPLE_TRACE != CEX_OFF || LIVENESS_COUNT > 0
  const struct state *previous;

  /* Index of the rule we took to reach this state. */
  uint64_t rule_taken;
#endif

  uintptr_t liveness[LIVENESS_COUNT / sizeof(uintptr_t) / CHAR_BIT +
    (LIVENESS_COUNT % sizeof(uintptr_t) == 0 &&
     LIVENESS_COUNT / sizeof(uintptr_t) % CHAR_BIT == 0 ? 0 : 1)];

#if BOUND > UINT64_MAX
  #error "no type large enough for state.bound"
#elif BOUND > UINT32_MAX
  uint64_t bound;
#elif BOUND > UINT16_MAX
  uint32_t bound;
#elif BOUND > UINT8_MAX
  uint16_t bound;
#elif BOUND > 0
  uint8_t bound;
#endif

  uint8_t data[STATE_SIZE_BYTES];
};

/*******************************************************************************
 * State allocator.                                                            *
 *                                                                             *
 * The following implements a simple bump allocator for states. The purpose of *
 * this (rather than simply mallocing individual states) is to speed up        *
 * allocation by taking global locks less frequently and decrease allocator    *
 * metadata overhead.                                                          *
 ******************************************************************************/

/* An initial size of thread-local allocator pools ~8MB. */
static _Thread_local size_t arena_count =
  (sizeof(struct state*) > 8 * 1024 * 1024)
    ? 1
    : (8 * 1024 * 1024 / sizeof(struct state*));

static _Thread_local struct state *arena_base;
static _Thread_local struct state *arena_limit;

static struct state *state_new(void) {

  if (arena_base == arena_limit) {
    /* Allocation pool is empty. We need to set up a new pool. */
    for (;;) {
      if (arena_count == 1) {
        arena_base = xmalloc(sizeof(*arena_base));
      } else {
        arena_base = calloc(arena_count, sizeof(*arena_base));
        if (arena_base == NULL) {
          /* Memory pressure high. Decrease our attempted allocation and try
           * again.
           */
          arena_count /= 2;
          continue;
        }
      }

      arena_limit = arena_base + arena_count;
      break;
    }
  }

  assert(arena_base != NULL);
  assert(arena_base != arena_limit);

  struct state *s = arena_base;
  arena_base++;
  return s;
}

static void state_free(struct state *s) {

  if (s == NULL) {
    return;
  }

  assert(s + 1 == arena_base);
  arena_base--;
}

/******************************************************************************/

/* Print a counterexample trace terminating at the given state. This function
 * assumes that the caller already holds print_mutex.
 */
static void print_counterexample(
  const struct state *NONNULL s __attribute__((unused)));

/* "Exit" the current thread. This takes into account which thread we are. I.e.
 * the correct way to exit the checker is for every thread to eventually call
 * this function.
 */
static _Noreturn int exit_with(int status);

static __attribute__((format(printf, 2, 3))) _Noreturn void error(
  const struct state *NONNULL s, const char *NONNULL fmt, ...) {

  unsigned long prior_errors = __atomic_fetch_add(&error_count, 1,
    __ATOMIC_SEQ_CST);

  if (prior_errors < MAX_ERRORS) {

    print_lock();

    va_list ap;
    va_start(ap, fmt);

    if (MACHINE_READABLE_OUTPUT) {
      printf("<error includes_trace=\"%s\">\n",
        (s == NULL || COUNTEREXAMPLE_TRACE == CEX_OFF) ? "false" : "true");

      printf("<message>");
      {
        va_list ap2;
        va_copy(ap2, ap);

        int size = vsnprintf(NULL, 0, fmt, ap2);
        va_end(ap2);
        if (size < 0) {
          fputs("vsnprintf failed", stderr);
          exit(EXIT_FAILURE);
        }

        char *buffer = xmalloc(size);
        if (vsnprintf(buffer, size, fmt, ap) != size) {
          fputs("vsnprintf failed", stderr);
          exit(EXIT_FAILURE);
        }

        xml_printf(buffer);
        free(buffer);
      }
      printf("</message>\n");

      if (s != NULL && COUNTEREXAMPLE_TRACE != CEX_OFF) {
        print_counterexample(s);
      }

      printf("</error>\n");

    } else {
      if (s != NULL) {
        printf("The following is the error trace for the error:\n\n");
      } else {
        printf("Result:\n\n");
      }

      printf("\t%s%s", red(), bold());
      vprintf(fmt, ap);
      printf("%s\n\n", reset());

      if (s != NULL && COUNTEREXAMPLE_TRACE != CEX_OFF) {
        print_counterexample(s);
        printf("End of the error trace.\n\n");
      }
    }

    va_end(ap);

    print_unlock();
  }

#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if (prior_errors < MAX_ERRORS - 1) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    assert(JMP_BUF_NEEDED && "longjmping without a setup jmp_buf");
    siglongjmp(checkpoint, 1);
  }

  exit_with(EXIT_FAILURE);
}

static void deadlock(const struct state *NONNULL s) {
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* error() longjmped back to us. */
      return;
    }
  }
  error(s, "deadlock");
}

static int state_cmp(const struct state *NONNULL a,
    const struct state *NONNULL b) {
  return memcmp(a->data, b->data, sizeof(a->data));
}

static bool state_eq(const struct state *NONNULL a,
    const struct state *NONNULL b) {
  return state_cmp(a, b) == 0;
}

static struct state *state_dup(const struct state *NONNULL s) {
  struct state *n = state_new();
  memcpy(n->data, s->data, sizeof(n->data));
#if COUNTEREXAMPLE_TRACE != CEX_OFF || LIVENESS_COUNT > 0
  n->previous = s;
#endif
#if BOUND > 0
  assert(s->bound < BOUND && "exceeding bounded exploration depth");
  n->bound = s->bound + 1;
#endif
  memset(n->liveness, 0, sizeof(n->liveness));
  return n;
}

static size_t state_hash(const struct state *NONNULL s) {
  return (size_t)MurmurHash64A(s->data, sizeof(s->data));
}

#if COUNTEREXAMPLE_TRACE != CEX_OFF
static __attribute__((unused)) size_t state_depth(
    const struct state *NONNULL s) {
#if BOUND > 0
  return (size_t)s->bound + 1;
#else
  size_t d = 0;
  while (s != NULL) {
    d++;
    s = s->previous;
  }
  return d;
#endif
}
#endif

/* A type-safe const cast. */
static __attribute__((unused)) struct state *state_drop_const(const struct state *s) {
  return (struct state*)s;
}

/* These functions are generated. */
static void state_canonicalise_heuristic(struct state *NONNULL s);
static void state_canonicalise_exhaustive(struct state *NONNULL s);

static void state_canonicalise(struct state *NONNULL s) {

  assert(s != NULL && "attempt to canonicalise NULL state");

  switch (SYMMETRY_REDUCTION) {

    case SYMMETRY_REDUCTION_OFF:
      break;

    case SYMMETRY_REDUCTION_HEURISTIC:
      state_canonicalise_heuristic(s);
      break;

    case SYMMETRY_REDUCTION_EXHAUSTIVE:
      state_canonicalise_exhaustive(s);
      break;

  }
}

/* This function is generated. */
static __attribute__((unused)) void state_print_field_offsets(void);

/* Print a state to stderr. This function is generated. This function assumes
 * that the caller already holds print_mutex.
 */
static __attribute__((unused)) void state_print(const struct state *previous,
  const struct state *NONNULL s);

/* Print the first rule that resulted in s. This function is generated. This
 * function assumes that the caller holds print_mutex.
 */
static __attribute__((unused)) void print_transition(
    const struct state *NONNULL s);

static void print_counterexample(
    const struct state *NONNULL s __attribute__((unused))) {

  assert(s != NULL && "missing state in request for counterexample trace");

#if COUNTEREXAMPLE_TRACE != CEX_OFF
  /* Construct an array of the states we need to print by walking backwards to
   * the initial starting state.
   */
  size_t trace_length = state_depth(s);

  const struct state **cex = xcalloc(trace_length, sizeof(cex[0]));

  {
    size_t i = trace_length - 1;
    for (const struct state *p = s; p != NULL; p = p->previous) {
      assert(i < trace_length && "error in counterexample trace traversal "
        "logic");
      cex[i] = p;
      i--;
    }
  }

  for (size_t i = 0; i < trace_length; i++) {

    const struct state *current = cex[i];
    const struct state *previous = i == 0 ? NULL : cex[i - 1];

    print_transition(current);

    if (MACHINE_READABLE_OUTPUT) {
      printf("<state>\n");
    }
    state_print(COUNTEREXAMPLE_TRACE == FULL ? NULL : previous, current);
    if (MACHINE_READABLE_OUTPUT) {
      printf("</state>\n");
    } else {
      printf("----------\n\n");
    }
  }

  free(cex);
#endif
}

struct handle {
  uint8_t *base;
  size_t offset;
  size_t width;
};

static struct handle handle_align(struct handle h) {

  ASSERT(h.offset < CHAR_BIT && "handle has an offset outside the base byte");

  size_t width = h.width + h.offset;
  if (width % 8 != 0) {
    width += 8 - width % 8;
  }

  return (struct handle){
    .base = h.base,
    .offset = 0,
    .width = width,
  };
}

static __attribute__((unused)) struct handle state_handle(
    const struct state *NONNULL s, size_t offset, size_t width) {

  assert(sizeof(s->data) * CHAR_BIT - width >= offset && "generating an out of "
    "bounds handle in state_handle()");

  return (struct handle){
    .base = (uint8_t*)s->data + offset / CHAR_BIT,
    .offset = offset % CHAR_BIT,
    .width = width,
  };
}

// TODO: The logic in this function is complex and fiddly. It would be desirable
// to have a proof in, e.g. Z3, that the manipulations it's doing actually yield
// the correct result.
static raw_value_t handle_read_raw(struct handle h) {

  // FIXME: When we get a user-configurable value type, users will be able to
  // cause this assertion to fail, so we should validate the necessary
  // conditions at code generation time.
  assert(h.width <= sizeof(raw_value_t) * 8 && "read of a handle to a value "
    "that is larger than our raw value type");

  ASSERT(h.width <= MAX_SIMPLE_WIDTH && "read of a handle that is larger than "
    "the maximum width of a simple type in this model");

  if (h.width == 0) {
    TRACE(TC_HANDLE_READS, "read value 0 from handle { %p, %zu, %zu }",
      h.base, h.offset, h.width);
    return 0;
  }

  /* Generate a handle that is offset- and width-aligned on byte boundaries.
   * Essentially, we widen the handle to align it. The motivation for this is
   * that we can only do byte-granularity reads, so we need to "over-read" if we
   * have an unaligned handle.
   */
  struct handle aligned = handle_align(h);
  ASSERT(aligned.offset == 0);

  /* The code below attempts to provide four alternatives for reading out the
   * bits corresponding to a value of simple type referenced by a handle, and to
   * give the compiler enough hints to steer it towards picking one of these and
   * removing the other three as dead code:
   *
   *   1. Read into a single 64-bit variable. Enabled when the maximum width for
   *      an unaligned handle spans 0 - 8 bytes.
   *   2. Read into two 64-bit variables and then combine the result using
   *      shifts and ORs. Enabled when the maximum width for an unaligned handle
   *      spans 9 - 16 bytes and the compiler does not provide the `__int128`
   *      type.
   *   3. Read into a single 128-bit variable. Enabled when the compiler does
   *      provide the `__int128` type and the maximum width for an unaligned
   *      handle spans 9 - 16 bytes.
   *   4. Read into two 128-bit chunks, and then combine the result using shifts
   *      and ORs. Enabled when the compiler provides the `__int128` type and
   *      the maximum width for an unaligned handle spans 17 - 32 bytes.
   */

#ifdef __SIZEOF_INT128__ /* if we have the type `__int128` */

  /* If a byte-unaligned value_t cannot be fully read into a single uint64_t
   * using byte-aligned reads...
   */
  if (MAX_SIMPLE_WIDTH > (sizeof(uint64_t) - 1) * 8) {

    /* Read the low double-word of this (possibly quad-word-sized) value. */
    unsigned __int128 low = 0;
    size_t low_size = aligned.width / 8;
    /* optimisation hint: */
    ASSERT(low_size <= sizeof(low) || MAX_SIMPLE_WIDTH > (sizeof(low) - 1) * 8);
    if (low_size > sizeof(low)) {
      low_size = sizeof(low);
    }
    {
      const uint8_t *src = aligned.base;
      memcpy(&low, src, low_size);
    }

    low >>= h.offset;

    size_t high_size = aligned.width / 8 - low_size;

    /* If the value could not be read into a single double-word... */
    ASSERT(high_size == 0 || MAX_SIMPLE_WIDTH > (sizeof(low) - 1) * 8);
    if (high_size != 0) {
      unsigned __int128 high = 0;
      const uint8_t *src = aligned.base + sizeof(low);
      memcpy(&high, src, high_size);

      high <<= sizeof(low) * 8 - h.offset;

      /* Combine the two halves into a single double-word. */
      low |= high;
    }

    if (h.width < sizeof(low) * 8) {
      unsigned __int128 mask = (((unsigned __int128)1) << h.width) - 1;
      low &= mask;
    }

    raw_value_t v = (raw_value_t)low;

    TRACE(TC_HANDLE_READS, "read value %" PRIRAWVAL " from handle { %p, %zu, %zu }",
      raw_value_to_string(v), h.base, h.offset, h.width);

    return v;
  }
#endif

  /* Read the low word of this (possibly two-word-sized) value. */
  uint64_t low = 0;
  size_t low_size = aligned.width / 8;
  /* optimisation hint: */
  ASSERT(low_size <= sizeof(low) || MAX_SIMPLE_WIDTH > (sizeof(low) - 1) * 8);
  if (low_size > sizeof(low)) {
    low_size = sizeof(low);
  }
  {
    const uint8_t *src = aligned.base;
    memcpy(&low, src, low_size);
  }

  low >>= h.offset;

  size_t high_size = aligned.width / 8 - low_size;

  /* If the value could not be read into a single word... */
  ASSERT(high_size == 0 || MAX_SIMPLE_WIDTH > (sizeof(low) - 1) * 8);
  if (high_size != 0) {
    uint64_t high = 0;
    const uint8_t *src = aligned.base + sizeof(low);
    memcpy(&high, src, high_size);

    high <<= sizeof(low) * 8 - h.offset;

    /* Combine the high and low words. Note that we know we can store the final
     * result in a single word because, if we've reached this point,
     * sizeof(value_t) <= sizeof(uint64_t).
     */
    low |= high;
  }

  if (h.width < sizeof(low) * 8) {
    uint64_t mask = (UINT64_C(1) << h.width) - 1;
    low &= mask;
  }

  raw_value_t v = (raw_value_t)low;

  TRACE(TC_HANDLE_READS, "read value %" PRIRAWVAL " from handle { %p, %zu, %zu }",
    raw_value_to_string(v), h.base, h.offset, h.width);

  return v;
}

static value_t decode_value(value_t lb, value_t ub, raw_value_t v) {

  value_t dest = 0;

  bool r __attribute__((unused)) = SUB(v, 1, &v) || ADD(v, lb, &dest)
    || dest < lb || dest > ub;

  ASSERT(!r && "read of out-of-range value");

  return dest;
}

static __attribute__((unused)) value_t handle_read(const char *NONNULL context,
    const char *rule_name, const char *NONNULL name,
    const struct state *NONNULL s, value_t lb, value_t ub, struct handle h) {

  assert(context != NULL);
  assert(name != NULL);

  /* If we happen to be reading from the current state, do a sanity check that
   * we're only reading within bounds.
   */
  assert((h.base != (uint8_t*)s->data /* not a read from the current state */
    || sizeof(s->data) * CHAR_BIT - h.width >= h.offset) /* in bounds */
    && "out of bounds read in handle_read()");

  raw_value_t dest = handle_read_raw(h);

  if (dest == 0) {
    error(s, "%sread of undefined value in %s%s%s", context, name,
      rule_name == NULL ? "" : " within ", rule_name == NULL ? "" : rule_name);
  }

  return decode_value(lb, ub, dest);
}

static void handle_write_raw(struct handle h, raw_value_t value) {

  assert(h.width <= sizeof(raw_value_t) * 8 && "write of a handle to a value "
    "that is larger than our raw value type");

  ASSERT(h.width <= MAX_SIMPLE_WIDTH && "write of a handle that is larger than "
    "the maximum width of a simple type in this model");

  TRACE(TC_HANDLE_WRITES, "writing value %" PRIRAWVAL " to handle { %p, %zu, %zu }",
    raw_value_to_string(value), h.base, h.offset, h.width);

  if (h.width == 0) {
    return;
  }

  /* Generate a offset- and width-aligned handle on byte boundaries. */
  struct handle aligned = handle_align(h);
  ASSERT(aligned.offset == 0);

#ifdef __SIZEOF_INT128__ /* if we have the type `__int128` */

  /* If a byte-unaligned value_t cannot be fully written within a single
   * byte-aligned uint64_t...
   */
  if (MAX_SIMPLE_WIDTH > (sizeof(uint64_t) - 1) * 8) {

    /* Read the low double-word of this region. */
    unsigned __int128 low = 0;
    size_t low_size = aligned.width / 8;
    ASSERT(low_size <= sizeof(low) || MAX_SIMPLE_WIDTH > (sizeof(low) - 1) * 8);
    if (low_size > sizeof(low)) {
      low_size = sizeof(low);
    }
    {
      const uint8_t *src = aligned.base;
      memcpy(&low, src, low_size);
    }

    {
      unsigned __int128 or_mask = ((unsigned __int128)value) << h.offset;
      if (low_size < sizeof(low)) {
        or_mask &= (((unsigned __int128)1) << (low_size * 8)) - 1;
      }
      unsigned __int128 and_mask = (((unsigned __int128)1) << h.offset) - 1;
      if (low_size < sizeof(low)) {
        size_t high_bits = aligned.width - h.offset - h.width;
        assert(high_bits >= 0);
        and_mask |= ((((unsigned __int128)1) << high_bits) - 1) << (low_size * 8 - high_bits);
      }

      low = (low & and_mask) | or_mask;
    }

    {
      uint8_t *dest = aligned.base;
      memcpy(dest, &low, low_size);
    }

    /* Now do the second double-word if necessary. */

    size_t high_size = aligned.width / 8 - low_size;

    ASSERT(high_size == 0 || MAX_SIMPLE_WIDTH > (sizeof(low) - 1) * 8);
    if (high_size != 0) {
      unsigned __int128 high = 0;
      {
        const uint8_t *src = aligned.base + sizeof(low);
        memcpy(&high, src, high_size);
      }

      {
        unsigned __int128 or_mask
          = ((unsigned __int128)value) >> (sizeof(low) * 8 - h.offset);
        unsigned __int128 and_mask
          = (~(unsigned __int128)0) & ~((((unsigned __int128)1) << (aligned.width - h.width)) - 1);

        high = (high & and_mask) | or_mask;
      }

      {
        uint8_t *dest = aligned.base + sizeof(low);
        memcpy(dest, &high, high_size);
      }
    }

    return;
  }
#endif

  /* Replicate the above logic for uint64_t. */

  uint64_t low = 0;
  size_t low_size = aligned.width / 8;
  ASSERT(low_size <= sizeof(low) || MAX_SIMPLE_WIDTH > (sizeof(low) - 1) * 8);
  if (low_size > sizeof(low)) {
    low_size = sizeof(low);
  }
  {
    const uint8_t *src = aligned.base;
    memcpy(&low, src, low_size);
  }

  {
    uint64_t or_mask = ((uint64_t)value) << h.offset;
    if (low_size < sizeof(low)) {
      or_mask &= (UINT64_C(1) << (low_size * 8)) - 1;
    }
    uint64_t and_mask = (UINT64_C(1) << h.offset) - 1;
    if (low_size < sizeof(low)) {
      size_t high_bits = aligned.width - h.offset - h.width;
      assert(high_bits >= 0);
      and_mask |= ((UINT64_C(1) << high_bits) - 1) << (low_size * 8 - high_bits);
    }

    low = (low & and_mask) | or_mask;
  }

  {
    uint8_t *dest = aligned.base;
    memcpy(dest, &low, low_size);
  }

  size_t high_size = aligned.width / 8 - low_size;

  ASSERT(high_size == 0 || MAX_SIMPLE_WIDTH > (sizeof(low) - 1) * 8);
  if (high_size != 0) {
    uint64_t high = 0;
    {
      const uint8_t *src = aligned.base + sizeof(low);
      memcpy(&high, src, high_size);
    }

    {
      uint64_t or_mask = ((uint64_t)value) >> (sizeof(low) * 8 - h.offset);
      uint64_t and_mask
        = (~UINT64_C(0)) & ~((UINT64_C(1) << (aligned.width - h.width)) - 1);

      high = (high & and_mask) | or_mask;
    }

    {
      uint8_t *dest = aligned.base + sizeof(low);
      memcpy(dest, &high, high_size);
    }
  }
}

static __attribute__((unused)) void handle_write(const char *NONNULL context,
    const char *rule_name, const char *NONNULL name,
    const struct state *NONNULL s, value_t lb, value_t ub, struct handle h,
    value_t value) {

  assert(context != NULL);
  assert(name != NULL);

  /* If we happen to be writing to the current state, do a sanity check that
   * we're only writing within bounds.
   */
  assert((h.base != (uint8_t*)s->data /* not a write to the current state */
    || sizeof(s->data) * CHAR_BIT - h.width >= h.offset) /* in bounds */
    && "out of bounds write in handle_write()");

  raw_value_t r;
  if (value < lb || value > ub || SUB(value, lb, &r) || ADD(r, 1, &r)) {
    error(s, "%swrite of out-of-range value into %s%s%s", context, name,
      rule_name == NULL ? "" : " within ", rule_name == NULL ? "" : rule_name);
  }

  handle_write_raw(h, r);
}

static __attribute__((unused)) void handle_zero(struct handle h) {

  uint8_t *p = h.base + h.offset / 8;

  /* Zero out up to a byte-aligned offset. */
  if (h.offset % 8 != 0) {
    uint8_t mask = (UINT8_C(1) << (h.offset % 8)) - 1;
    if (h.width < 8 - h.offset % 8) {
      mask |= UINT8_MAX & ~((UINT8_C(1) << (h.offset % 8 + h.width)) - 1);
    }
    *p &= mask;
    p++;
    if (h.width < 8 - h.offset % 8) {
      return;
    }
    h.width -= 8 - h.offset % 8;
  }

  /* Zero out as many bytes as we can. */
  memset(p, 0, h.width / 8);
  p += h.width / 8;
  h.width -= h.width / 8 * 8;

  /* Zero out the trailing bits in the final byte. */
  if (h.width > 0) {
    uint8_t mask = ~((UINT8_C(1) << h.width) - 1);
    *p &= mask;
  }
}

static __attribute__((unused)) void handle_copy(struct handle a,
    struct handle b) {

  ASSERT(a.width == b.width && "copying between handles of different sizes");

  /* FIXME: This does a bit-by-bit copy which almost certainly could be
   * accelerated by detecting byte-boundaries and complementary alignment and
   * then calling memcpy when possible.
   */

  for (size_t i = 0; i < a.width; i++) {

    uint8_t *dst = a.base + (a.offset + i) / 8;
    size_t dst_off = (a.offset + i) % 8;

    const uint8_t *src = b.base + (b.offset + i) / 8;
    size_t src_off = (b.offset + i) % 8;

    uint8_t or_mask = ((*src >> src_off) & UINT8_C(1)) << dst_off;
    uint8_t and_mask = ~(UINT8_C(1) << dst_off);

    *dst = (*dst & and_mask) | or_mask;
  }
}

static __attribute__((unused)) bool handle_eq(struct handle a,
    struct handle b) {

  ASSERT(a.width == b.width && "comparing handles of different sizes");

  /* FIXME: as with handle_copy, we do a bit-by-bit comparison which could be
   * made more efficient.
   */

  for (size_t i = 0; i < a.width; i++) {

    uint8_t *x = a.base + (a.offset + i) / 8;
    size_t x_off = (a.offset + i) % 8;
    bool x_bit = (*x >> x_off) & 0x1;

    const uint8_t *y = b.base + (b.offset + i) / 8;
    size_t y_off = (b.offset + i) % 8;
    bool y_bit = (*y >> y_off) & 0x1;

    if (x_bit != y_bit) {
      return false;
    }
  }

  return true;
}

static __attribute__((unused)) struct handle handle_narrow(struct handle h,
  size_t offset, size_t width) {

  ASSERT(h.offset + offset + width <= h.offset + h.width &&
    "narrowing a handle with values that actually expand it");

  size_t r __attribute__((unused));
  assert(!ADD(h.offset, offset, &r) && "narrowing handle overflows a size_t");

  return (struct handle){
    .base = h.base + (h.offset + offset) / CHAR_BIT,
    .offset = (h.offset + offset) % CHAR_BIT,
    .width = width,
  };
}

static __attribute__((unused)) struct handle handle_index(
    const char *NONNULL context, const char *rule_name,
    const char *NONNULL expr, const struct state *NONNULL s,
    size_t element_width, value_t index_min, value_t index_max,
    struct handle root, value_t index) {

  assert(expr != NULL);

  if (index < index_min || index > index_max) {
    error(s, "%sindex out of range in expression %s%s%s", context, expr,
      rule_name == NULL ? "" : " within ", rule_name == NULL ? "" : rule_name);
  }

  size_t r1, r2;
  if (SUB(index, index_min, &r1) || MUL(r1, element_width, &r2)) {
    error(s, "%soverflow when indexing array in expression %s%s%s", context,
      expr, rule_name == NULL ? "" : " within ",
      rule_name == NULL ? "" : rule_name);
  }

  size_t r __attribute__((unused));
  assert(!ADD(root.offset, r2, &r) && "indexing handle overflows a size_t");

  return (struct handle){
    .base = root.base + (root.offset + r2) / CHAR_BIT,
    .offset = (root.offset + r2) % CHAR_BIT,
    .width = element_width,
  };
}

static __attribute__((unused)) value_t handle_isundefined(struct handle h) {
  raw_value_t v = handle_read_raw(h);

  return v == 0;
}

/* Overflow-safe helpers for doing bounded arithmetic. */

static __attribute__((unused)) value_t add(const char *NONNULL context,
    const char *rule_name, const char *NONNULL expr,
    const struct state *NONNULL s, value_t a, value_t b) {

  assert(context != NULL);
  assert(expr != NULL);

  value_t r;
  if (ADD(a, b, &r)) {
    error(s, "%sinteger overflow in addition in expression %s%s%s", context,
      expr, rule_name == NULL ? "" : " within ",
      rule_name == NULL ? "" : rule_name);
  }
  return r;
}

static __attribute__((unused)) value_t sub(const char *NONNULL context,
    const char *rule_name, const char *NONNULL expr,
    const struct state *NONNULL s, value_t a, value_t b) {

  assert(context != NULL);
  assert(expr != NULL);

  value_t r;
  if (SUB(a, b, &r)) {
    error(s, "%sinteger overflow in subtraction in expression %s%s%s", context,
      expr, rule_name == NULL ? "" : " within ",
      rule_name == NULL ? "" : rule_name);
  }
  return r;
}

static __attribute__((unused)) value_t mul(const char *NONNULL context,
    const char *rule_name, const char *NONNULL expr,
    const struct state *NONNULL s, value_t a, value_t b) {

  assert(context != NULL);
  assert(expr != NULL);

  value_t r;
  if (MUL(a, b, &r)) {
    error(s, "%sinteger overflow in multiplication in expression %s%s%s",
      context, expr, rule_name == NULL ? "" : " within ",
      rule_name == NULL ? "" : rule_name);
  }
  return r;
}

static __attribute__((unused)) value_t divide(const char *NONNULL context,
    const char *rule_name, const char *NONNULL expr,
    const struct state *NONNULL s, value_t a, value_t b) {

  assert(context != NULL);
  assert(expr != NULL);

  if (b == 0) {
    error(s, "%sdivision by zero in expression %s%s%s", context, expr,
      rule_name == NULL ? "" : " within ", rule_name == NULL ? "" : rule_name);
  }

  if (VALUE_MIN != 0 && a == VALUE_MIN && b == (value_t)-1) {
    error(s, "%sinteger overflow in division in expression %s%s%s", context,
      expr, rule_name == NULL ? "" : " within ",
      rule_name == NULL ? "" : rule_name);
  }

  return a / b;
}

static __attribute__((unused)) value_t mod(const char *NONNULL context,
    const char *rule_name, const char *NONNULL expr,
    const struct state *NONNULL s, value_t a, value_t b) {

  assert(context != NULL);
  assert(expr != NULL);

  if (b == 0) {
    error(s, "%smodulus by zero in expression %s%s%s", context, expr,
      rule_name == NULL ? "" : " within ", rule_name == NULL ? "" : rule_name);
  }

  // Is INT_MIN % -1 UD? Reading the C spec I'm not sure.
  if (VALUE_MIN != 0 && a == VALUE_MIN && b == (value_t)-1) {
    error(s, "%sinteger overflow in modulo in expression %s%s%s",
      context, expr, rule_name == NULL ? "" : " within ",
      rule_name == NULL ? "" : rule_name);
  }

  return a % b;
}

static __attribute__((unused)) value_t negate(const char *NONNULL context,
    const char *rule_name, const char *NONNULL expr,
    const struct state *NONNULL s, value_t a) {

  assert(context != NULL);
  assert(expr != NULL);

  if (VALUE_MIN != 0 && a == VALUE_MIN) {
    error(s, "%sinteger overflow in negation in expression %s%s%s",
      context, expr, rule_name == NULL ? "" : " within ",
      rule_name == NULL ? "" : rule_name);
  }

  return -a;
}

/* A version of quicksort that operates on "schedules," arrays of indices that
 * serve as a proxy for the collection being sorted.
 */
static __attribute__((unused)) void sort(
  int (*NONNULL compare)(const struct state *s, size_t a, size_t b),
  size_t *NONNULL schedule, struct state *NONNULL s, size_t lower,
  size_t upper) {

  /* If we have nothing to sort, bail out. */
  if (lower >= upper) {
    return;
  }

  /* Use Hoare's partitioning algorithm to apply quicksort. */
  size_t i = lower - 1;
  size_t j = upper + 1;

  for (;;) {

    do {
      i++;
      assert(i >= lower && i <= upper && "out of bounds access in sort()");
    } while (compare(s, schedule[i], schedule[lower]) < 0);

    do {
      j--;
      assert(j >= lower && j <= upper && "out of bounds access in sort()");
    } while (compare(s, schedule[j], schedule[lower]) > 0);

    if (i >= j) {
      break;
    }

    /* Swap elements i and j. */
    size_t temp = schedule[i];
    schedule[i] = schedule[j];
    schedule[j] = temp;
  }

  sort(compare, schedule, s, lower, j);
  sort(compare, schedule, s, j + 1, upper);
}

/*******************************************************************************
 * State queue node                                                            *
 *                                                                             *
 * Queue nodes are 4K-sized, 4K-aligned linked-list nodes. They contain        *
 * pending states and then a pointer to the next node in the queue.            *
 ******************************************************************************/

struct queue_node {
  struct state *s[(4096 - sizeof(struct queue_node*)) / sizeof(struct state*)];
  struct queue_node *next;
};

_Static_assert(sizeof(struct queue_node) == 4096,
  "incorrect queue_node size calculation");

static struct queue_node *queue_node_new(void) {
  struct queue_node *p = NULL;

  int r = posix_memalign((void**)&p, sizeof(*p), sizeof(*p));

  assert((r == 0 || r == ENOMEM) && "invalid alignment to posix_memalign");

  if (r != 0) {
    oom();
  }

  memset(p, 0, sizeof(*p));

  return p;
}

static void queue_node_free(struct queue_node *p) {
  free(p);
}

/******************************************************************************/

/*******************************************************************************
 * Queue node handles                                                          *
 *                                                                             *
 * These are pointers to a member-aligned address within a queue_node. The     *
 * idea is that you can have a queue_handle_t pointing at either one of the    *
 * elements of the `s` member or at the chained `next` pointer. Since          *
 * queue_node pointers are always 4K-aligned, you can examine the low 12 bits  *
 * of the queue node handle to determine which member you are pointing at.     *
 ******************************************************************************/

typedef uintptr_t queue_handle_t;

static queue_handle_t queue_handle_from_node_ptr(const struct queue_node *n) {
  return (queue_handle_t)n;
}

static struct queue_node *queue_handle_base(queue_handle_t h) {
  return (struct queue_node*)(h - h % sizeof(struct queue_node));
}

static bool queue_handle_is_state_pptr(queue_handle_t h) {
  return h % sizeof(struct queue_node)
    < __builtin_offsetof(struct queue_node, next);
}

static struct state **queue_handle_to_state_pptr(queue_handle_t h) {
  assert(queue_handle_is_state_pptr(h) &&
    "invalid use of queue_handle_to_state_pptr");

  return (struct state**)h;
}

static struct queue_node **queue_handle_to_node_pptr(queue_handle_t h) {
  assert(!queue_handle_is_state_pptr(h) &&
    "invalid use of queue_handle_to_node_pptr");

  return (struct queue_node**)h;
}

static queue_handle_t queue_handle_next(queue_handle_t h) {
  return h + sizeof(struct state*);
}

/******************************************************************************/

/*******************************************************************************
 * Hazard pointers                                                             *
 *                                                                             *
 * The idea of "hazard pointers" comes from Maged Michael, "Hazard Pointers:   *
 * Safe Memory Reclamation for Lock-Free Objects" in TPDS 15(8) 2004. The      *
 * basic concept is to maintain a collection of safe-to-dereference pointers.  *
 * Before freeing a pointer, you look in this collection to see if it is in    *
 * use and you must always add a pointer to this collection before             *
 * dereferencing it. The finer details of how we keep this consistent and why  *
 * the size of this collection can be statically known ahead of time are a     *
 * little complicated, but the paper explains this in further detail.          *
 ******************************************************************************/

/* Queue node pointers currently safe to dereference. */
static const struct queue_node *hazarded[THREADS];

/* Protect a pointer that we wish to dereference. */
static __attribute__((unused)) void hazard(queue_handle_t h) {

  /* Find the queue node this handle lies within. */
  const struct queue_node *p = queue_handle_base(h);

  /* You can't protect the null pointer because it is invalid to dereference it.
   */
  assert(p != NULL && "attempt to hazard an invalid pointer");

  /* Each thread is only allowed a single hazarded pointer at a time. */
  assert(__atomic_load_n(&hazarded[thread_id], __ATOMIC_SEQ_CST) == NULL
    && "hazarding multiple pointers at once");

  __atomic_store_n(&hazarded[thread_id], p, __ATOMIC_SEQ_CST);
}

/* Drop protection on a pointer whose target we are done accessing. */
static __attribute__((unused)) void unhazard(queue_handle_t h) {

  /* Find the queue node this handle lies within. */
  const struct queue_node *p __attribute__((unused)) = queue_handle_base(h);

  assert(p != NULL && "attempt to unhazard an invalid pointer");

  assert(__atomic_load_n(&hazarded[thread_id], __ATOMIC_SEQ_CST) != NULL
    && "unhazarding a pointer when none are hazarded");

  assert(__atomic_load_n(&hazarded[thread_id], __ATOMIC_SEQ_CST) == p
    && "unhazarding a pointer that differs from the one hazarded");

  __atomic_store_n(&hazarded[thread_id], NULL, __ATOMIC_SEQ_CST);
}

/* Free a pointer or, if not possible, defer this to later. */
static __attribute__((unused)) void reclaim(queue_handle_t h) {

  /* Find the queue node this handle lies within. */
  struct queue_node *p = queue_handle_base(h);

  assert(p != NULL && "reclaiming a null pointer");

  /* The reclaimer is not allowed to be freeing something while also holding a
   * hazarded pointer.
   */
  assert(__atomic_load_n(&hazarded[thread_id], __ATOMIC_SEQ_CST) == NULL
    && "reclaiming a pointer while holding a hazarded pointer");

  /* Pointers that we failed to free initially because they were in use
   * (hazarded) at the time they were passed to reclaim().
   *
   * Why are we sure we will only ever have a maximum of `THREADS - 1` pointers
   * outstanding? Anything passed to reclaim() is expected to be
   * now-unreachable, so the only outstanding references to such are threads
   * racing with us. Because each thread can only have one hazarded pointer at a
   * time, the maximum number of in use pointers right now is `THREADS - 1`
   * (because the current thread does not have one).
   *
   * There is an edge case where another thread (1) held a hazarded pointer we
   * previously tried to reclaim and thus ended up on our deferred list, then
   * (2) in-between that time and now dropped this reference and acquired a
   * hazarded pointer to `p`. This still will not exceed our count of
   * `THREADS - 1` as the order in which we scan the deferred list and then add
   * `p` to it below ensures that we will discover the originally hazarded
   * pointer (now no longer conflicted) and clear its slot, leaving this
   * available for `p`.
   */
  static _Thread_local struct queue_node *deferred[THREADS - 1];

  /* First try to free any previously deferred pointers. */
  for (size_t i = 0; i < sizeof(deferred) / sizeof(deferred[0]); i++) {
    if (deferred[i] != NULL) {
      bool conflict = false;
      for (size_t j = 0; j < sizeof(hazarded) / sizeof(hazarded[0]); j++) {
        if (j == thread_id) {
          /* No need to check for conflicts with ourself. */
          assert(__atomic_load_n(&hazarded[j], __ATOMIC_SEQ_CST) == NULL);
          continue;
        }
        if (deferred[i] == __atomic_load_n(&hazarded[j], __ATOMIC_SEQ_CST)) {
          /* This pointer is in use by thread j. */
          conflict = true;
          break;
        }
      }
      if (!conflict) {
        queue_node_free(deferred[i]);
        deferred[i] = NULL;
      }
    }
  }

  /* Now deal with the pointer we were passed. The most likely case is that no
   * one else is using this pointer, so try this first.
   */
  bool conflict = false;
  for (size_t i = 0; i < sizeof(hazarded) / sizeof(hazarded[i]); i++) {
    if (i == thread_id) {
      /* No need to check for conflicts with ourself. */
      assert(__atomic_load_n(&hazarded[i], __ATOMIC_SEQ_CST) == NULL);
      continue;
    }
    if (p == __atomic_load_n(&hazarded[i], __ATOMIC_SEQ_CST)) {
      /* Bad luck :( */
      conflict = true;
      break;
    }
  }

  if (!conflict) {
    /* We're done! */
    queue_node_free(p);
    return;
  }

  /* If we reached here, we need to defer this reclamation to later. */
  for (size_t i = 0; i < sizeof(deferred) / sizeof(deferred[0]); i++) {
    if (deferred[i] == NULL) {
      deferred[i] = p;
      return;
    }
  }

  assert(!"deferred more than `THREADS` reclamations");
  __builtin_unreachable();
}

/******************************************************************************/

/*******************************************************************************
 * Atomic operations on double word values                                     *
 ******************************************************************************/

#ifdef __x86_64__
  /* It seems MOV on x86-64 is not guaranteed to be atomic on 128-bit naturally
   * aligned memory. The way to work around this is apparently the following
   * degenerate CMPXCHG.
   */
  #define atomic_read(p) __sync_val_compare_and_swap((p), 0, 0)
#else
  #define atomic_read(p) __atomic_load_n((p), __ATOMIC_SEQ_CST)
#endif

#ifdef __x86_64__
  /* As explained above, we need some extra gymnastics to avoid a call to
   * libatomic on x86-64.
   */
  #define atomic_write(p, v) \
    do { \
      __typeof__(p) _target = (p); \
      __typeof__(*(p)) _expected; \
      __typeof__(*(p)) _old = 0; \
      __typeof__(*(p)) _new = (v); \
      do { \
        _expected = _old; \
        _old = __sync_val_compare_and_swap(_target, _expected, _new); \
      } while (_expected != _old); \
    } while (0)
#else
  #define atomic_write(p, v) __atomic_store_n((p), (v), __ATOMIC_SEQ_CST)
#endif

#ifdef __x86_64__
  /* Make GCC >= 7.1 emit cmpxchg on x86-64. See
   * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80878.
   */
  #define atomic_cas(p, expected, new) \
    __sync_bool_compare_and_swap((p), (expected), (new))
#else
  #define atomic_cas(p, expected, new) \
    __atomic_compare_exchange_n((p), &(expected), (new), false, \
      __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
#endif

#ifdef __x86_64__
  /* Make GCC >= 7.1 emit cmpxchg on x86-64. See
   * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80878.
   */
  #define atomic_cas_val(p, expected, new) \
    __sync_val_compare_and_swap((p), (expected), (new))
#else
  #define atomic_cas_val(p, expected, new) \
    ({ \
      __typeof__(expected) _expected = (expected); \
      __atomic_compare_exchange_n((p), &(_expected), (new), false, \
        __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST); \
      _expected; \
    })
#endif

/******************************************************************************/

/*******************************************************************************
 * Double pointers                                                             *
 *                                                                             *
 * A scalar type that can store the value of two pointers.                     *
 ******************************************************************************/

#if __SIZEOF_POINTER__ <= 4
  typedef uint64_t double_ptr_t;
#elif __SIZEOF_POINTER__ <= 8
  typedef unsigned __int128 double_ptr_t;
#else
  #error "unexpected pointer size; what scalar type to use for double_ptr_t?"
#endif

static uintptr_t double_ptr_extract1(double_ptr_t p) {

  _Static_assert(sizeof(p) > sizeof(uintptr_t), "double_ptr_t is not big "
    "enough to fit a pointer");

  uintptr_t q;
  memcpy(&q, &p, sizeof(q));

  return q;
}

static uintptr_t double_ptr_extract2(double_ptr_t p) {

  _Static_assert(sizeof(p) >= 2 * sizeof(uintptr_t), "double_ptr_t is not big "
    "enough to fit two pointers");

  uintptr_t q;
  memcpy (&q, (unsigned char*)&p + sizeof(void*), sizeof(q));

  return q;
}

static double_ptr_t double_ptr_make(uintptr_t q1, uintptr_t q2) {

  double_ptr_t p = 0;

  _Static_assert(sizeof(p) >= 2 * sizeof(uintptr_t), "double_ptr_t is not big "
    "enough to fit two pointers");

  memcpy(&p, &q1, sizeof(q1));
  memcpy((unsigned char*)&p + sizeof(q1), &q2, sizeof(q2));

  return p;
}

/******************************************************************************/

/*******************************************************************************
 * State queue                                                                 *
 *                                                                             *
 * The following implements a per-thread queue for pending states. The only    *
 * supported operations are enqueueing and dequeueing states. A property we    *
 * maintain is that all states within all queues pass the current model's      *
 * invariants.                                                                 *
 ******************************************************************************/

static struct {
  double_ptr_t ends;
  size_t count;
} q[THREADS];

static size_t queue_enqueue(struct state *NONNULL s, size_t queue_id) {
  assert(queue_id < sizeof(q) / sizeof(q[0]) && "out of bounds queue access");

  /* Look up the tail of the queue. */

  double_ptr_t ends = atomic_read(&q[queue_id].ends);

retry:;
  queue_handle_t tail = double_ptr_extract2(ends);

  if (tail == 0) {
    /* There's nothing currently in the queue. */

    assert(double_ptr_extract1(ends) == 0 && "tail of queue 0 while head is "
      "non-0");

    struct queue_node *n = queue_node_new();
    n->s[0] = s;

    double_ptr_t new = double_ptr_make(queue_handle_from_node_ptr(n),
                                       queue_handle_from_node_ptr(n));

    double_ptr_t old = atomic_cas_val(&q[queue_id].ends, ends, new);
    if (old != ends) {
      /* Failed. */
      queue_node_free(n);
      ends = old;
      goto retry;
    }

  } else {
    /* The queue is non-empty, so we'll need to access the last element. */

    /* Try to protect our upcoming access to the tail. */
    hazard(tail);
    {
      double_ptr_t ends_check = atomic_read(&q[queue_id].ends);
      if (ends != ends_check) {
        /* Failed. Someone else modified the queue in the meantime. */
        unhazard(tail);
        ends = ends_check;
        goto retry;
      }
    }

    /* We've now notified other threads that we're going to be accessing the
     * tail, so we can safely dereference its pointer.
     */

    struct queue_node *new_node = NULL;
    queue_handle_t next_tail = queue_handle_next(tail);

    if (queue_handle_is_state_pptr(next_tail)) {
      /* There's an available slot in this queue node; no need to create a new
       * one.
       */

      {
        struct state **target = queue_handle_to_state_pptr(next_tail);
        struct state *null = NULL;
        if (!__atomic_compare_exchange_n(target, &null, s, false,
            __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
          /* Failed. Someone else enqueued before we could. */
          unhazard(tail);
          goto retry;
        }
      }

    } else {
      /* There's no remaining slot in this queue node. We'll need to create a
       * new (empty) queue node, add our state to this one and then append this
       * node to the queue.
       */

      /* Create the new node. */
      new_node = queue_node_new();
      new_node->s[0] = s;

      /* Try to update the chained pointer of the current tail to point to this
       * new node.
       */
      struct queue_node **target = queue_handle_to_node_pptr(next_tail);
      struct queue_node *null = NULL;
      if (!__atomic_compare_exchange_n(target, &null, new_node, false,
          __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
        /* Failed. Someone else enqueued before we could. */
        queue_node_free(new_node);
        unhazard(tail);
        goto retry;
      }

      /* We now need the tail to point at our new node. */
      next_tail = queue_handle_from_node_ptr(new_node);
    }

    queue_handle_t head = double_ptr_extract1(ends);
    double_ptr_t new = double_ptr_make(head, next_tail);

    /* Try to update the queue. */
    {
      double_ptr_t old = atomic_cas_val(&q[queue_id].ends, ends, new);
      if (old != ends) {
        /* Failed. Someone else dequeued before we could finish. We know the
         * operation that beat us was a dequeue and not an enqueue, because by
         * writing to next_tail we have prevented any other enqueue from
         * succeeding.
         */

        /* Undo the update of next_tail. We know this is safe (non-racy) because
         * no other enqueue can proceed and a dequeue will never look into
         * next_tail due to the way it handles the case when head == tail.
         */
        next_tail = queue_handle_next(tail);
        if (queue_handle_is_state_pptr(next_tail)) {
          /* We previously wrote into an existing queue node. */
          struct state **target = queue_handle_to_state_pptr(next_tail);
          struct state *temp = s;
          bool r __attribute__((unused)) = __atomic_compare_exchange_n(target,
            &temp, NULL, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
          assert(r && "undo of write to next_tail failed");
        } else {
          /* We previously wrote into a new queue node. */
          struct queue_node **target = queue_handle_to_node_pptr(next_tail);
          struct queue_node *temp = new_node;
          bool r __attribute__((unused)) = __atomic_compare_exchange_n(target,
            &temp, NULL, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
          assert(r && "undo of write to next_tail failed");

          queue_node_free(new_node);
        }

        unhazard(tail);
        ends = old;
        goto retry;
      }
    }

    /* Success! */

    unhazard(tail);
  }

  size_t count = __atomic_add_fetch(&q[queue_id].count, 1, __ATOMIC_SEQ_CST);

  TRACE(TC_QUEUE, "enqueued state %p into queue %zu, queue length is now %zu",
    s, queue_id, count);

  return count;
}

static const struct state *queue_dequeue(size_t *NONNULL queue_id) {
  assert(queue_id != NULL && *queue_id < sizeof(q) / sizeof(q[0]) &&
    "out of bounds queue access");

  const struct state *s = NULL;

  for (size_t attempts = 0; attempts < sizeof(q) / sizeof(q[0]); attempts++) {

    double_ptr_t ends = atomic_read(&q[*queue_id].ends);

retry:;
    queue_handle_t head = double_ptr_extract1(ends);

    if (head != 0) {
      /* This queue is non-empty. */

      /* Try to protect our upcoming accesses to the head. */
      hazard(head);
      {
        double_ptr_t ends_check = atomic_read(&q[*queue_id].ends);
        if (ends != ends_check) {
          /* Failed. Someone else updated the queue. */
          unhazard(head);
          ends = ends_check;
          goto retry;
        }
      }

      queue_handle_t tail = double_ptr_extract2(ends);

      double_ptr_t new;
      if (head == tail) {
        /* There is only a single element in the queue. We will need to update
         * both head and tail.
         */
        new = double_ptr_make(0, 0);
      } else if (queue_handle_is_state_pptr(head)) {
        /* There are multiple elements in the queue; we can deal only with the
         * head.
         */
        new = double_ptr_make(queue_handle_next(head), tail);
      } else {
        /* The head of the queue is the end of a queue node. I.e. the only thing
         * remaining in this queue node is the chained pointer to the next queue
         * node.
         */

        /* Load the next queue node. */
        struct queue_node **n = queue_handle_to_node_pptr(head);
        struct queue_node *new_head = __atomic_load_n(n, __ATOMIC_SEQ_CST);
        new = double_ptr_make(queue_handle_from_node_ptr(new_head), tail);

        /* Try to replace the current head with the next node. */
        double_ptr_t old = atomic_cas_val(&q[*queue_id].ends, ends, new);
        /* Either way, now we'll need to retry, but if we succeeded we also need
         * to free the queue node we just removed.
         */
        unhazard(head);
        if (old == ends) {
          /* Succeeded. */
          reclaim(head);
        }
        ends = old;
        goto retry;
      }

      /* Try to remove the head. */
      {
        double_ptr_t old = atomic_cas_val(&q[*queue_id].ends, ends, new);
        if (old != ends) {
          /* Failed. Someone else either enqueued or dequeued. */
          unhazard(head);
          ends = old;
          goto retry;
        }
      }

      /* We now have either a pointer to a state or we've just removed an empty
       * queue node that was the last in the queue.
       */

      if (queue_handle_is_state_pptr(head)) {
        struct state **st = queue_handle_to_state_pptr(head);
        s = *st;
      }

      unhazard(head);

      if (head == tail || !queue_handle_is_state_pptr(head)) {
        reclaim(head);
      }

      if (s == NULL) {
        /* Move to the next queue to try. */
        *queue_id = (*queue_id + 1) % (sizeof(q) / sizeof(q[0]));
        continue;
      }

      size_t count = __atomic_sub_fetch(&q[*queue_id].count, 1,
        __ATOMIC_SEQ_CST);

      TRACE(TC_QUEUE, "dequeued state %p from queue %zu, queue length is now "
        "%zu", s, *queue_id, count);

      return s;
    }

    /* Move to the next queue to try. */
    *queue_id = (*queue_id + 1) % (sizeof(q) / sizeof(q[0]));
  }

  return s;
}

/******************************************************************************/

/*******************************************************************************
 * Reference counted pointers                                                  *
 *                                                                             *
 * These are capable of encapsulating any generic pointer (void*). Note that   *
 * we rely on the existence of double-word atomics. On x86-64, you need to     *
 * use compiler flag '-mcx16' to get an efficient 128-bit cmpxchg.             *
 *                                                                             *
 * Of these functions, only the following are thread safe:                     *
 *                                                                             *
 *   * refcounted_ptr_get                                                      *
 *   * refcounted_ptr_peek                                                     *
 *   * refcounted_ptr_put                                                      *
 *   * refcounted_ptr_set                                                      *
 *                                                                             *
 * The caller is expected to coordinate with other threads to exclude them     *
 * operating on the relevant refcounted_ptr_t when using one of the other      *
 * functions:                                                                  *
 *                                                                             *
 *   * refcounted_ptr_shift                                                    *
 *                                                                             *
 ******************************************************************************/

struct refcounted_ptr {
  void *ptr;
  size_t count;
};

#if __SIZEOF_POINTER__ <= 4
  typedef uint64_t refcounted_ptr_t;
#elif __SIZEOF_POINTER__ <= 8
  typedef unsigned __int128 refcounted_ptr_t;
#else
  #error "unexpected pointer size; what scalar type to use for refcounted_ptr_t?"
#endif

_Static_assert(sizeof(struct refcounted_ptr) <= sizeof(refcounted_ptr_t),
  "refcounted_ptr does not fit in a refcounted_ptr_t, which we need to operate "
  "on it atomically");

static void refcounted_ptr_set(refcounted_ptr_t *NONNULL p, void *ptr) {

#ifndef NDEBUG
  /* Read the current state of the pointer. */
  refcounted_ptr_t old = atomic_read(p);
  struct refcounted_ptr p2;
  memcpy(&p2, &old, sizeof(old));
  assert(p2.count == 0 && "overwriting a pointer source while someone still "
    "has a reference to this pointer");
#endif

  /* Set the current source pointer with no outstanding references. */
  struct refcounted_ptr p3;
  p3.ptr = ptr;
  p3.count = 0;

  /* Commit the result. */
  refcounted_ptr_t new;
  memcpy(&new, &p3, sizeof(p3));
  atomic_write(p, new);
}

static void *refcounted_ptr_get(refcounted_ptr_t *NONNULL p) {

  refcounted_ptr_t old, new;
  void *ret;
  bool r;

  do {

    /* Read the current state of the pointer. */
    old = atomic_read(p);
    struct refcounted_ptr p2;
    memcpy(&p2, &old, sizeof(old));

    /* Take a reference to it. */
    p2.count++;
    ret = p2.ptr;

    /* Try to commit our results. */
    memcpy(&new, &p2, sizeof(new));
    r = atomic_cas(p, old, new);
  } while (!r);

  return ret;
}

static void refcounted_ptr_put(refcounted_ptr_t *NONNULL p,
  void *ptr __attribute__((unused))) {

  refcounted_ptr_t old, new;
  bool r;

  do {

    /* Read the current state of the pointer. */
    old = atomic_read(p);
    struct refcounted_ptr p2;
    memcpy(&p2, &old, sizeof(old));

    /* Release our reference to it. */
    ASSERT(p2.ptr == ptr && "releasing a reference to a pointer after someone "
      "has changed the pointer source");
    ASSERT(p2.count > 0 && "releasing a reference to a pointer when it had no "
      "outstanding references");
    p2.count--;

    /* Try to commit our results. */
    memcpy(&new, &p2, sizeof(new));
    r = atomic_cas(p, old, new);
  } while (!r);
}

static void *refcounted_ptr_peek(refcounted_ptr_t *NONNULL p) {

  /* Read out the state of the pointer. This rather unpleasant expression is
   * designed to emit an atomic load at a smaller granularity than the entire
   * refcounted_ptr structure. Because we only need the pointer -- and not the
   * count -- we can afford to just atomically read the first word.
   */
  void *ptr = __atomic_load_n(
    (void**)((void*)p + __builtin_offsetof(struct refcounted_ptr, ptr)),
    __ATOMIC_SEQ_CST);

  return ptr;
}

static void refcounted_ptr_shift(refcounted_ptr_t *NONNULL current,
    refcounted_ptr_t *NONNULL next) {

  /* None of the operations in this function are performed atomically because we
   * assume the caller has synchronised with other threads via other means.
   */

  /* The pointer we're about to overwrite should not be referenced. */
  struct refcounted_ptr p __attribute__((unused));
  memcpy(&p, current, sizeof(*current));
  ASSERT(p.count == 0 && "overwriting a pointer that still has outstanding "
    "references");

  /* Shift the next value into the current pointer. */
  *current = *next;

  /* Blank the value we just shifted over. */
  *next = 0;
}

/******************************************************************************/

/*******************************************************************************
 * Thread rendezvous support                                                   *
 ******************************************************************************/

static pthread_mutex_t rendezvous_lock; /* mutual exclusion mechanism for below. */
static pthread_cond_t rendezvous_cond;  /* sleep mechanism for below. */
static size_t running_count = 1;            /* how many threads are opted in to rendezvous? */
static size_t rendezvous_pending = 1;   /* how many threads are opted in and not sleeping? */

static void rendezvous_init(void) {
  int r = pthread_mutex_init(&rendezvous_lock, NULL);
  if (r != 0) {
    fprintf(stderr, "pthread_mutex_init failed: %s\n", strerror(r));
    exit(EXIT_FAILURE);
  }

  r = pthread_cond_init(&rendezvous_cond, NULL);
  if (r != 0) {
    fprintf(stderr, "pthread_cond_init failed: %s\n", strerror(r));
    exit(EXIT_FAILURE);
  }
}

/* Call this at the start of a rendezvous point.
 *
 * This is a low level function, not expected to be directly used outside of the
 * context of the rendezvous implementation.
 *
 * @return True if the caller was the last to arrive and henceforth dubbed the
 *   'leader'.
 */
static bool rendezvous_arrive(void) {
  int r __attribute__((unused)) = pthread_mutex_lock(&rendezvous_lock);
  assert(r == 0);

  /* Take a token from the rendezvous down-counter. */
  assert(rendezvous_pending > 0);
  rendezvous_pending--;

  /* If we were the last to arrive then it was our arrival that dropped the
   * counter to zero.
   */
  return rendezvous_pending == 0;
}

/* Call this at the end of a rendezvous point.
 *
 * This is a low level function, not expected to be directly used outside of the
 * context of the rendezvous implementation.
 *
 * @param leader Whether the caller is the 'leader'. If you call this when you
 *   are the 'leader' it will unblock all 'followers' at the rendezvous point.
 * @param action Optional code for the leader to run.
 */
static void rendezvous_depart(bool leader, void (*action)(void)) {
  int r __attribute((unused));

  if (leader) {
    if (action != NULL) {
      action();
    }

    /* Reset the counter for the next rendezvous. */
    assert(rendezvous_pending == 0 && "a rendezvous point is being exited "
      "while some participating threads have yet to arrive");
    rendezvous_pending = running_count;

    /* Wake up the 'followers'. */
    r = pthread_cond_broadcast(&rendezvous_cond);
    assert(r == 0);

  } else {

    /* Wait on the 'leader' to wake us up. */
    r = pthread_cond_wait(&rendezvous_cond, &rendezvous_lock);
    assert(r == 0);
  }

  r = pthread_mutex_unlock(&rendezvous_lock);
  assert(r == 0);
}

/* Exposed friendly function for performing a rendezvous. */
static void rendezvous(void (*action)(void)) {
  bool leader = rendezvous_arrive();
  if (leader) {
    TRACE(TC_SET, "arrived at rendezvous point as leader");
  }
  rendezvous_depart(leader, action);
}

/* Remove the caller from the pool of threads who participate in this
 * rendezvous.
 */
static void rendezvous_opt_out(void (*action)(void)) {

retry:;

  /* "Arrive" at the rendezvous to decrement the count of outstanding threads.
   */
  bool leader = rendezvous_arrive();

  if (leader && running_count > 1) {
    /* We unfortunately opted out of this rendezvous while the remaining threads
     * were arriving at one and we were the last to arrive. Let's pretend we are
     * participating in the rendezvous and unblock them.
     */
    rendezvous_depart(true, action);

    /* Re-attempt opting-out. */
    goto retry;
  }

  /* Remove ourselves from the known threads. */
  assert(running_count > 0);
  running_count--;

  int r __attribute__((unused)) = pthread_mutex_unlock(&rendezvous_lock);
  assert(r == 0);
}

/******************************************************************************/

/*******************************************************************************
 * 'Slots', an opaque wrapper around a state pointer                           *
 *                                                                             *
 * See usage of this in the state set below for its purpose.                   *
 ******************************************************************************/

typedef uintptr_t slot_t;

static __attribute__((const)) slot_t slot_empty(void) {
  return 0;
}

static __attribute__((const)) bool slot_is_empty(slot_t s) {
  return s == slot_empty();
}

static __attribute__((const)) bool slot_is_tombstone(slot_t s) {
  return (s & 0x1) == 0x1;
}

static slot_t slot_bury(slot_t s) {
  ASSERT(!slot_is_tombstone(s));
  return s | 0x1;
}

static struct state *slot_to_state(slot_t s) {
  ASSERT(!slot_is_empty(s));
  ASSERT(!slot_is_tombstone(s));
  return (struct state*)s;
}

static slot_t state_to_slot(const struct state *s) {
  return (slot_t)s;
}

/******************************************************************************/

/*******************************************************************************
 * State set                                                                   *
 *                                                                             *
 * The following implementation provides a set for storing the seen states.    *
 * There is no support for testing whether something is in the set or for      *
 * removing elements, only thread-safe insertion of elements.                  *
 ******************************************************************************/

enum { INITIAL_SET_SIZE_EXPONENT = sizeof(unsigned long long) * 8 - 1 -
  __builtin_clzll(SET_CAPACITY / sizeof(struct state*) / sizeof(struct state)) };

struct set {
  slot_t *bucket;
  size_t size_exponent;
};

/* Some utility functions for dealing with exponents. */

static size_t set_size(const struct set *NONNULL set) {
  return ((size_t)1) << set->size_exponent;
}

static size_t set_index(const struct set *NONNULL set, size_t index) {
  return index & (set_size(set) - 1);
}

/* The states we have encountered. This collection will only ever grow while
 * checking the model. Note that we have a global reference-counted pointer and
 * a local bare pointer. See below for an explanation.
 */
static refcounted_ptr_t global_seen;
static _Thread_local struct set *local_seen;

/* Number of elements in the global set (i.e. occupancy). */
static size_t seen_count;

/* The "next" 'global_seen' value. See below for an explanation. */
static refcounted_ptr_t next_global_seen;

/* Now the explanation I teased... When the set capacity exceeds a threshold
 * (see 'set_expand' related logic below) it is expanded and the reference
 * tracking within the 'refcounted_ptr_t's comes in to play. We need to allocate
 * a new seen set ('next_global_seen'), copy over all the elements (done in
 * 'set_migrate'), and then "shift" the new set to become the current set. The
 * role of the reference counts of both 'global_seen' and 'next_global_seen' in
 * all of this is to detect when the last thread releases its reference to the
 * old seen set and hence can deallocate it.
 */

/* The next chunk to migrate from the old set to the new set. What exactly a
 * "chunk" is is covered in 'set_migrate'.
 */
static size_t next_migration;

/* A mechanism for synchronisation in 'set_expand'. */
static pthread_mutex_t set_expand_mutex;

static void set_expand_lock(void) {
  if (THREADS > 1) {
    int r __attribute__((unused)) = pthread_mutex_lock(&set_expand_mutex);
    ASSERT(r == 0);
  }
}

static void set_expand_unlock(void) {
  if (THREADS > 1) {
    int r __attribute__((unused)) = pthread_mutex_unlock(&set_expand_mutex);
    ASSERT(r == 0);
  }
}


static void set_init(void) {

  if (THREADS > 1) {
    int r = pthread_mutex_init(&set_expand_mutex, NULL);
    if (r < 0) {
      fprintf(stderr, "pthread_mutex_init failed: %s\n", strerror(r));
      exit(EXIT_FAILURE);
    }
  }

  /* Allocate the set we'll store seen states in at some conservative initial
   * size.
   */
  struct set *set = xmalloc(sizeof(*set));
  set->size_exponent = INITIAL_SET_SIZE_EXPONENT;
  set->bucket = xcalloc(set_size(set), sizeof(set->bucket[0]));

  /* Stash this somewhere for threads to later retrieve it from. Note that we
   * initialize its reference count to zero as we (the setup logic) are not
   * using it beyond this function.
   */
  refcounted_ptr_set(&global_seen, set);
}

static void set_thread_init(void) {
  /* Take a local reference to the global seen set. */
  local_seen = refcounted_ptr_get(&global_seen);
}

static void set_update(void) {
  /* Guard against the case where we've been called from exit_with() and we're
   * not finishing a migration, but just opting out of the rendezvous protocol.
   */
  if (refcounted_ptr_peek(&next_global_seen) != NULL) {

    /* Clean up the old set. Note that we are using a pointer we have already
     * given up our reference count to here, but we rely on the caller to ensure
     * this access is safe.
     */
    free(local_seen->bucket);
    free(local_seen);

    /* Reset migration state for the next time we expand the set. */
    next_migration = 0;

    /* Update the global pointer to the new set. We know all the above
     * migrations have completed and no one needs the old set.
     */
    refcounted_ptr_shift(&global_seen, &next_global_seen);
  }
}

static void set_migrate(void) {

  TRACE(TC_SET, "assisting in set migration...");

  /* Size of a migration chunk. Threads in this function grab a chunk at a time
   * to migrate.
   */
  enum { CHUNK_SIZE = 4096 / sizeof(local_seen->bucket[0]) /* slots */ };

  /* Take a pointer to the target set for the migration. */
  struct set *next = refcounted_ptr_get(&next_global_seen);

  for (;;) {

    size_t chunk = __atomic_fetch_add(&next_migration, 1, __ATOMIC_SEQ_CST);
    size_t start = chunk * CHUNK_SIZE;
    size_t end = start + CHUNK_SIZE;

    /* Bail out if we've finished migrating all of the set. */
    if (start >= set_size(local_seen)) {
      break;
    }

    // TODO: The following algorithm assumes insertions can collide. That is, it
    // operates atomically on slots because another thread could be migrating
    // and also targeting the same slot. If we were to more closely wick to the
    // Maier design this would not be required.

    for (size_t i = start; i < end; i++) {
retry:;

      /* Retrieve the slot element and try to mark it as migrated. */
      slot_t s = __atomic_load_n(&local_seen->bucket[i], __ATOMIC_SEQ_CST);
      ASSERT(!slot_is_tombstone(s) && "attempted double slot migration");
      if (!__atomic_compare_exchange_n(&local_seen->bucket[i], &s, slot_bury(s),
          false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
        goto retry;
      }

      /* If the current slot contained a state, rehash it and insert it into the
       * new set. Note we don't need to do any state comparisons because we know
       * everything in the old set is unique.
       */
      if (!slot_is_empty(s)) {
        size_t index = set_index(next, state_hash(slot_to_state(s)));
        for (size_t j = index; ; j = set_index(next, j + 1)) {
          slot_t expected = slot_empty();
          if (__atomic_compare_exchange_n(&next->bucket[j], &expected, s, false,
              __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
            /* Found an empty slot and inserted successfully. */
            break;
          }
        }
      }
    }

  }

  /* Release our reference to the old set now we're done with it. */
  refcounted_ptr_put(&global_seen, local_seen);

  /* Now we need to make sure all the threads get to this point before any one
   * thread leaves. The purpose of this is to guarantee we only ever have at
   * most two seen sets "in flight". Without this rendezvous, one thread could
   * race ahead, fill the new set, and then decide to expand again while some
   * are still working on the old set. It's possible to make such a scheme work
   * but the synchronisation requirements just seem too complicated.
   */
  rendezvous(set_update);

  /* We're now ready to resume model checking. Note that we already have a
   * (reference counted) pointer to the now-current global seen set, so we don't
   * need to take a fresh reference to it.
   */
  local_seen = next;
}

static void set_expand(void) {

  /* Using double-checked locking, we look to see if someone else has already
   * started expanding the set. We do this by first checking before acquiring
   * the set mutex, and then later again checking after we've acquired the
   * mutex. The idea here is that, with multiple threads, you'll frequently find
   * someone else beat you to expansion and you can jump straight to helping
   * them with migration without having the expense of acquiring the set mutex.
   */
  if (THREADS > 1 && refcounted_ptr_peek(&next_global_seen) != NULL) {
    /* Someone else already expanded it. Join them in the migration effort. */
    TRACE(TC_SET, "attempted expansion failed because another thread got there "
      "first");
    set_migrate();
    return;
  }

  set_expand_lock();

  /* Check again, as described above. */
  if (THREADS > 1 && refcounted_ptr_peek(&next_global_seen) != NULL) {
    set_expand_unlock();
    TRACE(TC_SET, "attempted expansion failed because another thread got there "
      "first");
    set_migrate();
    return;
  }

  TRACE(TC_SET, "expanding set from %zu slots to %zu slots...",
    (((size_t)1) << local_seen->size_exponent) / sizeof(slot_t),
    (((size_t)1) << (local_seen->size_exponent + 1)) / sizeof(slot_t));

  /* Create a set of double the size. */
  struct set *set = xmalloc(sizeof(*set));
  set->size_exponent = local_seen->size_exponent + 1;
  set->bucket = xcalloc(set_size(set), sizeof(set->bucket[0]));

  /* Advertise this as the newly expanded global set. */
  refcounted_ptr_set(&next_global_seen, set);

  /* We now need to migrate all slots from the old set to the new one, but we
   * can do this multithreaded.
   */
  set_expand_unlock();
  set_migrate();
}

static bool set_insert(struct state *NONNULL s, size_t *NONNULL count) {

restart:;

  if (__atomic_load_n(&seen_count, __ATOMIC_SEQ_CST) * 100
      / set_size(local_seen) >= SET_EXPAND_THRESHOLD)
    set_expand();

  size_t index = set_index(local_seen, state_hash(s));

  size_t attempts = 0;
  for (size_t i = index; attempts < set_size(local_seen); i = set_index(local_seen, i + 1)) {

    /* Guess that the current slot is empty and try to insert here. */
    slot_t c = slot_empty();
    if (__atomic_compare_exchange_n(&local_seen->bucket[i], &c,
        state_to_slot(s), false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
      /* Success */
      *count = __atomic_add_fetch(&seen_count, 1, __ATOMIC_SEQ_CST);
      TRACE(TC_SET, "added state %p, set size is now %zu", s, *count);

      /* The maximum possible size of the seen state set should be constrained
       * by the number of possible states based on how many bits we are using to
       * represent the state data.
       */
      if (STATE_SIZE_BITS < sizeof(size_t) * CHAR_BIT) {
        assert(*count <= 1 << STATE_SIZE_BITS && "seen set size "
          "exceeds total possible number of states");
      }

      return true;
    }

    if (slot_is_tombstone(c)) {
      /* This slot has been migrated. We need to rendezvous with other migrating
       * threads and restart our insertion attempt on the newly expanded set.
       */
      set_migrate();
      goto restart;
    }

    /* If we find this already in the set, we're done. */
    if (state_eq(s, slot_to_state(c))) {
      TRACE(TC_SET, "skipped adding state %p that was already in set", s);
      return false;
    }

    attempts++;
  }

  /* If we reach here, the set is full. Expand it and retry the insertion. */
  set_expand();
  return set_insert(s, count);
}

/* Find an existing element in the set.
 *
 * Why would you ever want to do this? If you already have the state, why do you
 * want to find a copy of it? The answer is for liveness information. When
 * checking liveness properties, a duplicate of your current state that is
 * already contained in the state set might know some of the liveness properties
 * are satisfied that your current state considers unknown.
 */
static const struct state *set_find(const struct state *NONNULL s) {

  assert(s != NULL);

  size_t index = set_index(local_seen, state_hash(s));

  size_t attempts = 0;
  for (size_t i = index; attempts < set_size(local_seen); i = set_index(local_seen, i + 1)) {

    slot_t slot = __atomic_load_n(&local_seen->bucket[i], __ATOMIC_SEQ_CST);

    /* This function is only expected to be called during the final liveness
     * scan, in which all threads participate. So we should never encounter a
     * set undergoing migration.
     */
    ASSERT(!slot_is_tombstone(slot)
      && "tombstone encountered during final phase");

    if (slot_is_empty(slot)) {
      /* reached the end of the linear block in which this state could lie */
      break;
    }

    const struct state *n = slot_to_state(slot);
    ASSERT(n != NULL && "null pointer stored in state set");

    if (state_eq(s, n)) {
      /* found */
      return n;
    }

    attempts++;
  }

  /* not found */
  return NULL;
}

/******************************************************************************/

static time_t START_TIME;

static unsigned long long gettime() {
  return (unsigned long long)(time(NULL) - START_TIME);
}

/* Set one of the liveness bits (i.e. mark the matching property as 'hit') in a
 * state and all its predecessors.
 */
static __attribute__((unused)) void mark_liveness(struct state *NONNULL s,
    size_t index, bool shared) {

  assert(s != NULL);
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(index < sizeof(s->liveness) * CHAR_BIT
    && "out of range liveness write");
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif

  size_t word_index = index / (sizeof(s->liveness[0]) * CHAR_BIT);
  size_t bit_index = index % (sizeof(s->liveness[0]) * CHAR_BIT);

  uintptr_t previous_value;
  uintptr_t *target = &s->liveness[word_index];
  uintptr_t mask = ((uintptr_t)1) << bit_index;

  if (shared) {
    /* If this state is shared (accessible by other threads) we need to operate
     * on its liveness data atomically.
     */
    previous_value = __atomic_fetch_or(target, mask, __ATOMIC_SEQ_CST);
  } else {
    /* Otherwise we can use a cheaper ordinary OR. */
    previous_value = *target;
    *target |= mask;
  }

  /* The following looks a little odd. Why do we only initialise this pointer
   * properly when we have liveness properties? The answer is that the
   * `previous` pointer in the state struct only exists when we have liveness
   * properties, so the access would cause a compile error if we did it
   * unconditionally. We leave the rest of this function visible to the compiler
   * even when we have no liveness properties to confirm it is syntactically
   * valid.
   */
  struct state *previous = NULL;
#if LIVENESS_COUNT > 0
  /* Cheat a little and cast away the constness of the previous state for which
   * we may need to update liveness data.
   */
  previous = (struct state*)s->previous;
#endif

  /* If the given bit was already set, we know all the predecessors of this
   * state have already had their corresponding bit marked. However, if it was
   * not we now need to recurse to mark them. Note that we assume any
   * predecessors of this state are globally visible and hence shared. The
   * recursion depth here can be indeterminately deep, but we assume the
   * compiler can tail-optimise this call.
   */
  if ((previous_value & mask) != mask && previous != NULL) {
    mark_liveness(previous, index, true);
  }
}

/* Whether we know all liveness properties are satisfied for a given state. */
static bool known_liveness(const struct state *NONNULL s) {

  assert(s != NULL);

#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  for (size_t i = 0; i < sizeof(s->liveness) / sizeof(s->liveness[0]); i++) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif

    uintptr_t word = __atomic_load_n(&s->liveness[i], __ATOMIC_SEQ_CST);

    for (size_t j = 0; j < sizeof(s->liveness[0]) * CHAR_BIT; j++) {
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
      if (i * sizeof(s->liveness[0]) * CHAR_BIT + j >= LIVENESS_COUNT) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }

      if (!((word >> j) & 0x1)) {
        return false;
      }
    }
  }

  return true;
}

/* Learn new liveness information about the state `s` from its successor. Note
 * that typically `successor->previous != s` because `successor` is actually one
 * of the de-duped aliases of the original successor to `s`.
 *
 * @param s State to learn information about
 * @param successor Successor to s
 * @return Number of new liveness information facts learnt
 */
static unsigned long learn_liveness(struct state *NONNULL s,
    const struct state *NONNULL successor) {

  assert(s != NULL);
  assert(successor != NULL);

  unsigned long new_info = 0;

#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  for (size_t i = 0; i < sizeof(s->liveness) / sizeof(s->liveness[0]); i++) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif

    uintptr_t word_src = __atomic_load_n(&successor->liveness[i],
      __ATOMIC_SEQ_CST);
    uintptr_t word_dst = __atomic_load_n(&s->liveness[i], __ATOMIC_SEQ_CST);

    for (size_t j = 0; j < sizeof(s->liveness[0]) * CHAR_BIT; j++) {
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
      if (i * sizeof(s->liveness[0]) * CHAR_BIT + j >= LIVENESS_COUNT) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }

      bool live_src = !!((word_src >> j) & 0x1);
      bool live_dst = !!((word_dst >> j) & 0x1);

      if (!live_dst && live_src) {
        mark_liveness(s, i * sizeof(s->liveness[0]) * CHAR_BIT + j, true);
        new_info++;
      }
    }
  }

  return new_info;
}

/* Prototypes for generated functions. */
static void init(void);
static _Noreturn void explore(void);
static void check_liveness_final(void);
static unsigned long check_liveness_summarise(void);

static int exit_with(int status) {

  /* Opt out of the thread-wide rendezvous protocol. */
  refcounted_ptr_put(&global_seen, local_seen);
  rendezvous_opt_out(set_update);
  local_seen = NULL;

  /* Make fired rule count visible globally. */
  rules_fired[thread_id] = rules_fired_local;

  if (thread_id == 0) {
    /* We are the initial thread. Wait on the others before exiting. */
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
    for (size_t i = 0; phase == RUN &&
         i < sizeof(threads) / sizeof(threads[0]); i++) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
      void *ret;
      int r = pthread_join(threads[i], &ret);
      if (r != 0) {
        print_lock();
        fprintf(stderr, "failed to join thread: %s\n", strerror(r));
        print_unlock();
        continue;
      }
      status |= (int)(intptr_t)ret;
    }

    /* We're now single-threaded again. */

    /* Reacquire a pointer to the seen set. Note that this may not be the same
     * value as what we previously had in local_seen because the other threads
     * may have expanded and migrated the seen set in the meantime.
     */
    local_seen = refcounted_ptr_get(&global_seen);

    if (error_count == 0) {
      /* If we didn't see any other errors, print cover information. */
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
      for (size_t i = 0; i < sizeof(covers) / sizeof(covers[0]); i++) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        if (MACHINE_READABLE_OUTPUT) {
          printf("<cover_result message=\"");
          xml_printf(COVER_MESSAGES[i]);
          printf("\" count=\"%" PRIuMAX "\"/>\n", covers[i]);
        }
        if (covers[i] == 0) {
          if (!MACHINE_READABLE_OUTPUT) {
            printf("\t%s%scover \"%s\" not hit%s\n", red(), bold(),
              COVER_MESSAGES[i], reset());
          }
          error_count++;
          status = EXIT_FAILURE;
        } else if (!MACHINE_READABLE_OUTPUT) {
          printf("\t%s%scover \"%s\" hit %" PRIuMAX " times%s\n", green(),
            bold(), COVER_MESSAGES[i], covers[i], reset());
        }
      }
    }

    /* If we have liveness properties to assess and have seen no previous
     * errors, do a final check of them now.
     */
    if (LIVENESS_COUNT > 0 && error_count == 0) {
      check_liveness_final();

      unsigned long failed = check_liveness_summarise();
      if (failed > 0) {
        error_count += failed;
        status = EXIT_FAILURE;
      }
    }

    if (!MACHINE_READABLE_OUTPUT) {
      printf("\n"
             "==========================================================================\n"
             "\n"
             "Status:\n"
             "\n");
      if (error_count == 0) {
        printf("\t%s%sNo error found.%s\n", green(), bold(), reset());
      } else {
        printf("\t%s%s%lu error(s) found.%s\n", red(), bold(), error_count, reset());
      }
      printf("\n");
    }

    /* Calculate the total number of rules fired. */
    uintmax_t fire_count = 0;
    for (size_t i = 0; i < sizeof(rules_fired) / sizeof(rules_fired[0]); i++) {
      fire_count += rules_fired[i];
    }

    /* Paranoid check that we didn't miscount during set insertions/expansions.
     */
#ifndef NDEBUG
    size_t count = 0;
    for (size_t i = 0; i < set_size(local_seen); i++) {
      if (!slot_is_empty(local_seen->bucket[i])) {
        count++;
      }
    }
#endif
    assert(count == seen_count && "seen set count is inconsistent at exit");

    if (MACHINE_READABLE_OUTPUT) {
      printf("<summary states=\"%zu\" rules_fired=\"%" PRIuMAX "\" errors=\"%lu\" "
        "duration_seconds=\"%llu\"/>\n", seen_count, fire_count, error_count,
        gettime());
      printf("</rumur_run>\n");
    } else {
      printf("State Space Explored:\n"
             "\n"
             "\t%zu states, %" PRIuMAX " rules fired in %llus.\n",
             seen_count, fire_count, gettime());
    }

    exit(status);
  } else {
    pthread_exit((void*)(intptr_t)status);
  }
}

static void *thread_main(void *arg) {

  /* Initialize (thread-local) thread identifier. */
  thread_id = (size_t)(uintptr_t)arg;

  set_thread_init();

  explore();
}

static void start_secondary_threads(void) {

  /* XXX: Kind of hacky. We've left the rendezvous down-counter at 1 until now
   * in case we triggered a rendezvous before starting the other threads (it
   * *can* happen). We bump it immediately -- i.e. before starting *any* of the
   * secondary threads -- because any of them could race us and trigger a
   * rendezvous before we exit the below loop and they need to know about the
   * thundering herd bearing down on them. It is safe to do this without holding
   * rendezvous_lock because we are still single threaded at this point.
   */
  assert(running_count == 1);
  running_count = THREADS;
  assert(rendezvous_pending == 1);
  rendezvous_pending = THREADS;

#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  for (size_t i = 0; i < sizeof(threads) / sizeof(threads[0]); i++) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    int r = pthread_create(&threads[i], NULL, thread_main,
      (void*)(uintptr_t)(i + 1));
    if (r != 0) {
      fprintf(stderr, "pthread_create failed: %s\n", strerror(r));
      exit(EXIT_FAILURE);
    }
  }
}

int main(void) {

  if (COLOR == AUTO)
    istty = isatty(STDOUT_FILENO) != 0;

  /* We don't need to read anything from stdin, so discard it. */
  (void)fclose(stdin);

  sandbox();

  if (MACHINE_READABLE_OUTPUT) {
    printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
           "<rumur_run>\n"
           "<information state_size_bits=\"%zu\" state_size_bytes=\"%zu\" "
      "hash_table_slots=\"%zu\"/>\n", (size_t)STATE_SIZE_BITS,
      (size_t)STATE_SIZE_BYTES, ((size_t)1) << INITIAL_SET_SIZE_EXPONENT);
  } else {
    printf("Memory usage:\n"
           "\n"
           "\t* The size of each state is %zu bits (rounded up to %zu bytes).\n"
           "\t* The size of the hash table is %zu slots.\n"
           "\n",
           (size_t)STATE_SIZE_BITS, (size_t)STATE_SIZE_BYTES,
           ((size_t)1) << INITIAL_SET_SIZE_EXPONENT);
  }

#ifndef NDEBUG
  state_print_field_offsets();
#endif

  START_TIME = time(NULL);

  rendezvous_init();

  set_init();

  set_thread_init();

  init();

  if (!MACHINE_READABLE_OUTPUT) {
    printf("Progress Report:\n\n");
  }

  explore();
}
