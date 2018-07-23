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

/* Generic support for maximum and minimum values of types. This is useful for,
 * e.g. size_t, where we don't properly have SIZE_MAX and SIZE_MIN.
 */
#define MIN(type) _Generic((type)1,                                            \
  int8_t:   INT8_MIN,                                                          \
  int16_t:  INT16_MIN,                                                         \
  int32_t:  INT32_MIN,                                                         \
  int64_t:  INT64_MIN,                                                         \
  uint8_t:  (uint8_t)0,                                                        \
  uint16_t: (uint16_t)0,                                                       \
  uint32_t: (uint32_t)0,                                                       \
  uint64_t: (uint64_t)0)
#define MAX(type) _Generic((type)1,                                            \
  int8_t:   INT8_MAX,                                                          \
  int16_t:  INT16_MAX,                                                         \
  int32_t:  INT32_MAX,                                                         \
  int64_t:  INT64_MAX,                                                         \
  uint8_t:  UINT8_MAX,                                                         \
  uint16_t: UINT16_MAX,                                                        \
  uint32_t: UINT32_MAX,                                                        \
  uint64_t: UINT64_MAX)

/* Abstraction over the type we use for scalar values. Other code should be
 * agnostic to what the underlying type is, so if you are porting this code to a
 * future platform where you need a wider type, modifying these lines should be
 * enough.
 */
typedef int64_t value_t;
#define PRIVAL PRId64
#define VALUE_MAX INT64_MAX
#define VALUE_MIN INT64_MIN
#define VALUE_C INT64_C

/* XXX: intypes.h does not seem to give us this. */
#ifndef SIZE_C
  #define SIZE_C(x) _Generic((size_t)1,                                        \
    unsigned: x ## u,                                                          \
    unsigned long: x ## ul,                                                    \
    unsigned long long: x ## ull)
#endif

/* A more powerful assert that treats the assertion as an assumption when
 * assertions are disabled.
 */
#ifndef NDEBUG
  #define ASSERT(expr) assert(expr)
#else
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

/* A word about atomics... There are three different atomic operation mechanisms
 * used in this code and it may not immediately be obvious why one was not
 * sufficient. The three are:
 *
 *   1. C11 atomics: used for variables that are consistently accessed with
 *      atomic semantics. This mechanism is simple, concise and standardised.
 *   2. GCC __atomic built-ins: Used for variables that are sometimes accessed
 *      with atomic semantics and sometimes as regular memory operations. The
 *      C11 atomics cannot give us this and the __atomic built-ins are
 *      implemented by the major compilers.
 *   3. GCC __sync built-ins: used for 128-bit atomic accesses on x86-64. It
 *      seems the __atomic built-ins do not result in a CMPXCHG instruction, but
 *      rather in a less efficient library call. See
 *      https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80878.
 */

/* Number of threads currently running. This is only necessary for mechanisms
 * like 'rendezvous' that require knowing how many threads have not yet exited.
 */
static atomic_size_t running_count = 1; // FIXME: we should initialise this at runtime to THREADS

/* Identifier of the current thread. This counts up from 0 and thus is suitable
 * to use for, e.g., indexing into arrays. The initial thread has ID 0.
 */
static _Thread_local size_t thread_id;

/* The threads themselves. Note that we have no element for the initial thread,
 * so *your* thread is 'threads[thread_id - 1]'.
 */
static pthread_t threads[THREADS - 1];

/* Whether we've encountered an error or not. If a thread sees this set, they
 * should attempt to exit gracefully as soon as possible.
 */
static atomic_bool done;

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

/*******************************************************************************
 * Cross-platform semaphores.                                                  *
 *                                                                             *
 * Sigh, Apple.                                                                *
 ******************************************************************************/

#ifdef __APPLE__
  #include <dispatch/dispatch.h>
#else
  #include <semaphore.h>
#endif

#ifdef __APPLE__
  typedef dispatch_semaphore_t semaphore_t;
#else
  typedef sem_t semaphore_t;
#endif

static int semaphore_init(semaphore_t *sem, unsigned int value) {
#ifdef __APPLE__
  *sem = dispatch_semaphore_create((long)value);
  return 0;
#else
  return sem_init(sem, 0, value);
#endif
}

static int semaphore_wait(semaphore_t *sem) {
#ifdef __APPLE__
  return dispatch_semaphore_wait(*sem, DISPATCH_TIME_FOREVER);
#else
  return sem_wait(sem);
#endif
}

static int semaphore_post(semaphore_t *sem) {
#ifdef __APPLE__
  (void)dispatch_semaphore_signal(*sem);
  return 0;
#else
  return sem_post(sem);
#endif
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

/* Supporting for tracing specific operations. This can be enabled during
 * checker generation with '--trace ...' and is useful for debugging Rumur
 * itself.
 */
static __attribute__((format(printf, 2, 3))) void trace(
  enum trace_category_t category, const char *fmt, ...) {

  if (category & TRACES_ENABLED) {
    va_list ap;
    va_start(ap, fmt);

    print_lock();

    (void)fprintf(stderr, "%sTRACE%s:", yellow(), reset());
    (void)vfprintf(stderr, fmt, ap);
    (void)fprintf(stderr, "\n");

    print_unlock();
    va_end(ap);
  }
}

/* The state of the current model. */
struct state {
  const struct state *previous;

  uint8_t data[STATE_SIZE_BYTES];
};

/* Print a counterexample trace terminating at the given state. This function
 * assumes that the caller already holds print_mutex.
 */
static unsigned print_counterexample(const struct state *s);

/* "Exit" the current thread. This takes into account which thread we are. I.e.
 * the correct way to exit the checker is for every thread to eventually call
 * this function.
 */
static _Noreturn int exit_with(int status);

static __attribute__((format(printf, 2, 3))) _Noreturn void error(
  const struct state *s, const char *fmt, ...) {

  bool expected = false;
  if (atomic_compare_exchange_strong(&done, &expected, true)) {

    print_lock();

    if (s != NULL) {
      fprintf(stderr, "The following is the error trace for the error:\n\n");
    } else {
      fprintf(stderr, "Result:\n\n");
    }

    fprintf(stderr, "\t%s%s", red(), bold());
    va_list ap, ap2;
    va_start(ap, fmt);
    va_copy(ap2, ap);
    (void)vfprintf(stderr, fmt, ap);
    fprintf(stderr, "%s\n\n", reset());
    va_end(ap);

    if (s != NULL) {
      print_counterexample(s);
      fprintf(stderr, "End of the error trace.\n\n");
    }

    fprintf(stderr, "==========================================================================\n"
                    "\n"
                    "Result:\n"
                    "\n"
                    "\t%s%s", red(), bold());
    vfprintf(stderr, fmt, ap2);
    va_end(ap2);
    fprintf(stderr, "%s\n\n", reset());

    print_unlock();
  }
  exit_with(EXIT_FAILURE);
}

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

static struct state *state_new(void) {
  return xcalloc(1, sizeof(struct state));
}

static int state_cmp(const struct state *a, const struct state *b) {
  return memcmp(a->data, b->data, sizeof(a->data));
}

static bool state_eq(const struct state *a, const struct state *b) {
  return state_cmp(a, b) == 0;
}

static struct state *state_dup(const struct state *s) {
  struct state *n = xmalloc(sizeof(*n));
  memcpy(n->data, s->data, sizeof(n->data));
  n->previous = s;
  return n;
}

static size_t state_hash(const struct state *s) {
  return (size_t)XXH64(s->data, sizeof(s->data), 0);
}

/* Print a state to stderr. This function is generated. This function assumes
 * that the caller already holds print_mutex.
 */
static void state_print(const struct state *s);

static unsigned print_counterexample(const struct state *s) {

  if (s == NULL) {
    return 0;
  }

  /* Recurse so that we print the states in reverse-linked order, which
   * corresponds to the order in which they were traversed.
   */
  unsigned step = print_counterexample(s->previous) + 1;

  state_print(s);
  fprintf(stderr, "----------\n\n");
  return step;
}

struct handle {
  uint8_t *base;
  size_t offset;
  size_t width;
};

static __attribute__((unused)) bool handle_aligned(struct handle h) {
  return h.offset % 8 == 0 && h.width % 8 == 0;
}

static struct handle handle_align(struct handle h) {

  size_t offset = h.offset - (h.offset % 8);
  size_t width = h.width + (h.offset % 8);
  if (width % 8 != 0) {
    width += 8 - width % 8;
  }

  return (struct handle){
    .base = h.base,
    .offset = offset,
    .width = width,
  };
}

struct handle state_handle(const struct state *s, size_t offset, size_t width) {

  assert(sizeof(s->data) * CHAR_BIT - width >= offset && "generating an out of "
    "bounds handle in state_handle()");

  return (struct handle){
    .base = (uint8_t*)s->data,
    .offset = offset,
    .width = width,
  };
}

static unsigned __int128 handle_extract(struct handle h) {

  ASSERT(handle_aligned(h) && "extraction of unaligned handle");

  unsigned __int128 v = 0;
  for (size_t i = 0; i < h.width / 8; i++) {
    unsigned __int128 byte = ((unsigned __int128)*(h.base + h.offset / 8 + i)) << (i * 8);
    v |= byte;
  }

  return v;
}

static void handle_insert(struct handle h, unsigned __int128 v) {

  ASSERT(handle_aligned(h) && "insertion to unaligned handle");

  for (size_t i = 0; i < h.width / 8; i++) {
    *(h.base + h.offset / 8 + i) = (uint8_t)(v >> (i * 8));
  }
}

static value_t handle_read_raw(struct handle h) {
  static_assert(sizeof(unsigned __int128) > sizeof(value_t),
    "handle_read_raw() is implemented by reading data into a 128-bit scalar, "
    "potentially reading more than the width of a value. Value type is larger "
    "than 128 bits which prevents this.");

  if (h.width == 0) {
    trace(TC_HANDLE_READS, "read value %" PRIVAL " from handle { %p, %zu, %zu }",
      (value_t)0, h.base, h.offset, h.width);
    return 0;
  }

  struct handle aligned = handle_align(h);
  unsigned __int128 v = handle_extract(aligned);
  v >>= h.offset % 8;
  v &= (((unsigned __int128)1) << h.width) - 1;

  value_t dest = (value_t)v;

  trace(TC_HANDLE_READS, "read value %" PRIVAL " from handle { %p, %zu, %zu }",
    dest, h.base, h.offset, h.width);

  return dest;
}

static value_t decode_value(value_t lb, value_t ub, value_t v) {

  value_t dest = v;

  bool r __attribute__((unused)) = __builtin_sub_overflow(dest, 1, &dest) ||
    __builtin_add_overflow(dest, lb, &dest) || dest < lb || dest > ub;

  ASSERT(!r && "read of out-of-range value");

  return dest;
}

static __attribute__((unused)) value_t handle_read(const struct state *s,
    value_t lb, value_t ub, struct handle h) {

  /* If we happen to be reading from the current state, do a sanity check that
   * we're only reading within bounds.
   */
  assert((h.base != (uint8_t*)s->data /* not a read from the current state */
    || sizeof(s->data) * CHAR_BIT - h.width >= h.offset) /* in bounds */
    && "out of bounds read in handle_read()");

  value_t dest = handle_read_raw(h);

  if (dest == 0) {
    error(s, "read of undefined value");
  }

  return decode_value(lb, ub, dest);
}

static void handle_write_raw(struct handle h, value_t value) {

  trace(TC_HANDLE_WRITES, "writing value %" PRIVAL " to handle { %p, %zu, %zu }",
    value, h.base, h.offset, h.width);

  if (h.width == 0) {
    return;
  }

  /* Extract the byte-aligned region in which we need to write this value. */
  struct handle aligned = handle_align(h);
  unsigned __int128 v = handle_extract(aligned);

  size_t bit_offset = h.offset % 8;
  static const unsigned __int128 one = 1;
  unsigned __int128 low_mask = bit_offset == 0 ? 0 : (one << bit_offset) - 1;
  unsigned __int128 value_mask = ((one << (bit_offset + h.width)) - 1) & ~low_mask;

  /* Write new value into the relevant slice in the middle of the extracted
   * data.
   */
  v = (v & ~value_mask) | (((unsigned __int128)value) << bit_offset);

  /* Write back this data into the target location. */
  handle_insert(aligned, v);
}

static __attribute__((unused)) void handle_write(const struct state *s,
    value_t lb, value_t ub, struct handle h, value_t value) {

  static_assert(sizeof(unsigned __int128) > sizeof(value_t),
    "handle_write() is implemented by reading data into a 128-bit scalar and "
    "then operating on it using 128-bit operations. Value type is larger than "
    "128 bits which prevents this.");

  /* If we happen to be writing to the current state, do a sanity check that
   * we're only writing within bounds.
   */
  assert((h.base != (uint8_t*)s->data /* not a write to the current state */
    || sizeof(s->data) * CHAR_BIT - h.width >= h.offset) /* in bounds */
    && "out of bounds write in handle_write()");

  if (value < lb || value > ub || __builtin_sub_overflow(value, lb, &value) ||
      __builtin_add_overflow(value, 1, &value)) {
    error(s, "write of out-of-range value");
  }

  handle_write_raw(h, value);
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

static __attribute__((unused)) struct handle handle_narrow(struct handle h,
  size_t offset, size_t width) {

  ASSERT(h.offset + offset + width <= h.offset + h.width &&
    "narrowing a handle with values that actually expand it");

  size_t r __attribute__((unused));
  assert(!__builtin_add_overflow(h.offset, offset, &r) &&
    "narrowing handle overflows a size_t");

  return (struct handle){
    .base = h.base,
    .offset = h.offset + offset,
    .width = width,
  };
}

static __attribute__((unused)) struct handle handle_index(const struct state *s,
  size_t element_width, value_t index_min, value_t index_max,
  struct handle root, value_t index) {

  if (index < index_min || index > index_max) {
    error(s, "index out of range");
  }

  size_t r1, r2;
  if (__builtin_sub_overflow(index, index_min, &r1) ||
      __builtin_mul_overflow(r1, element_width, &r2)) {
    error(s, "overflow when indexing array");
  }

  size_t r __attribute__((unused));
  assert(!__builtin_add_overflow(root.offset, r2, &r) &&
    "indexing handle overflows a size_t");

  return (struct handle){
    .base = root.base,
    .offset = root.offset + r2,
    .width = element_width,
  };
}

/* Overflow-safe helpers for doing bounded arithmetic. The compiler built-ins
 * used are implemented in modern GCC and Clang. If you're using another
 * compiler, you'll have to implement these yourself.
 */

static __attribute__((unused)) value_t add(const struct state *s,
  value_t lower_bound, value_t upper_bound, value_t a, value_t b) {

  value_t r;
  if (__builtin_add_overflow(a, b, &r)) {
    error(s, "integer overflow in addition");
  }
  if (r < lower_bound || r > upper_bound) {
    error(s, "result %" PRIVAL " of addition out of range", r);
  }
  return r;
}

static __attribute__((unused)) value_t sub(const struct state *s,
  value_t lower_bound, value_t upper_bound, value_t a, value_t b) {

  value_t r;
  if (__builtin_sub_overflow(a, b, &r)) {
    error(s, "integer overflow in subtraction");
  }
  if (r < lower_bound || r > upper_bound) {
    error(s, "result %" PRIVAL " of subtraction out of range", r);
  }
  return r;
}

static __attribute__((unused)) value_t mul(const struct state *s,
  value_t lower_bound, value_t upper_bound, value_t a, value_t b) {

  value_t r;
  if (__builtin_mul_overflow(a, b, &r)) {
    error(s, "integer overflow in multiplication");
  }
  if (r < lower_bound || r > upper_bound) {
    error(s, "result %" PRIVAL " of multiplication out of range", r);
  }
  return r;
}

static __attribute__((unused)) value_t divide(const struct state *s,
  value_t lower_bound, value_t upper_bound, value_t a, value_t b) {

  if (b == 0) {
    error(s, "division by zero");
  }

  if (a == VALUE_MIN && b == -1) {
    error(s, "integer overflow in division");
  }

  value_t r = a / b;
  if (r < lower_bound || r > upper_bound) {
    error(s, "result %" PRIVAL " of division out of range", r);
  }

  return r;
}

static __attribute__((unused)) value_t mod(const struct state *s,
  value_t lower_bound, value_t upper_bound, value_t a, value_t b) {

  if (b == 0) {
    error(s, "modulus by zero");
  }

  // Is INT64_MIN % -1 UD? Reading the C spec I'm not sure.
  if (a == VALUE_MIN && b == -1) {
    error(s, "integer overflow in modulo");
  }

  value_t r = a % b;
  if (r < lower_bound || r > upper_bound) {
    error(s, "result %" PRIVAL " of modulo out of range", r);
  }

  return r;
}

static __attribute__((unused)) value_t negate(const struct state *s,
  value_t lower_bound, value_t upper_bound, value_t a) {

  if (a == VALUE_MIN) {
    error(s, "integer overflow in negation");
  }

  value_t r = -a;
  if (r < lower_bound || r > upper_bound) {
    error(s, "result %" PRIVAL " of negation out of range", r);
  }

  return r;
}

/* A version of quicksort that operates on "schedules," arrays of indices that
 * serve as a proxy for the collection being sorted.
 */
static __attribute__((unused)) void sort(
  int (*compare)(const struct state *s, size_t a, size_t b),
  size_t *schedule, struct state *s, size_t lower, size_t upper) {

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
 * State queue                                                                 *
 *                                                                             *
 * The following implements a per-thread queue for pending states. The only    *
 * supported operations are enqueueing and dequeueing states. A property we    *
 * maintain is that all states within all queues pass the current model's      *
 * invariants.                                                                 *
 ******************************************************************************/

struct queue_node {
  struct state *s;
  struct queue_node *next;
};

static struct {
  pthread_mutex_t lock;
  struct queue_node *head;
  size_t count;
} q;

static void queue_init(void) {
  int r = pthread_mutex_init(&q.lock, NULL);
  if (r < 0) {
    fprintf(stderr, "pthread_mutex_init failed: %s\n", strerror(r));
    exit(EXIT_FAILURE);
  }
}

size_t queue_enqueue(struct state *s) {
  struct queue_node *n = xmalloc(sizeof(*n));
  n->s = s;

  int r __attribute__((unused)) = pthread_mutex_lock(&q.lock);
  ASSERT(r == 0);

  n->next = q.head;
  q.head = n;
  q.count++;

  trace(TC_QUEUE, "enqueued state %p, queue length is now %zu", s, q.count);

  size_t count = q.count;

  r = pthread_mutex_unlock(&q.lock);
  ASSERT(r == 0);

  return count;
}

const struct state *queue_dequeue(void) {

  const struct state *s = NULL;

  int r __attribute__((unused)) = pthread_mutex_lock(&q.lock);
  ASSERT(r == 0);

  struct queue_node *n = q.head;
  if (n != NULL) {
    q.head = n->next;
    q.count--;
    trace(TC_QUEUE, "dequeued state %p, queue length is now %zu", n->s, q.count);
  }

  r = pthread_mutex_unlock(&q.lock);
  ASSERT(r == 0);

  if (n != NULL) {
    s = n->s;
    free(n);
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
 *   * refcounted_ptr_put                                                      *
 *                                                                             *
 * The caller is expected to coordinate with other threads to exclude them     *
 * operating on the relevant refcounted_ptr_t when using one of the other      *
 * functions:                                                                  *
 *                                                                             *
 *   * refcounted_ptr_set                                                      *
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

static_assert(sizeof(struct refcounted_ptr) <= sizeof(refcounted_ptr_t),
  "refcounted_ptr does not fit in a refcounted_ptr_t, which we need to operate "
  "on it atomically");

static void refcounted_ptr_set(refcounted_ptr_t *p, void *ptr) {

  /* Read the current state of the pointer. Note, we don't bother doing this
   * atomically as it's only for debugging and no one else should be using the
   * pointer source right now.
   */
  struct refcounted_ptr p2;
  memcpy(&p2, p, sizeof(*p));
  ASSERT(p2.count == 0 && "overwriting a pointer source while someone still "
    "has a reference to this pointer");

  /* Set the current source pointer with no outstanding references. */
  p2.ptr = ptr;
  p2.count = 0;

  /* Commit the result. Again, we do not operate atomically because no one else
   * should be using the pointer source.
   */
  memcpy(p, &p2, sizeof(*p));
}

static void *refcounted_ptr_get(refcounted_ptr_t *p) {

  refcounted_ptr_t old, new;
  void *ret;
  bool r;

  do {

    /* Read the current state of the pointer. */
#ifdef __x86_64__
    /* It seems MOV on x86-64 is not guaranteed to be atomic on 128-bit
     * naturally aligned memory. The way to work around this is apparently the
     * following degenerate CMPXCHG.
     */
    old = __sync_val_compare_and_swap(p, *p, *p);
#else
    old = __atomic_load_n(p, __ATOMIC_SEQ_CST);
#endif
    struct refcounted_ptr p2;
    memcpy(&p2, &old, sizeof(old));

    /* Take a reference to it. */
    p2.count++;
    ret = p2.ptr;

    /* Try to commit our results. */
    memcpy(&new, &p2, sizeof(new));
#ifdef __x86_64__
    /* Make GCC >= 7.1 emit cmpxchg on x86-64. See
     * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80878.
     */
    r = __sync_bool_compare_and_swap(p, old, new);
#else
    r = __atomic_compare_exchange_n(p, &old, new, false, __ATOMIC_SEQ_CST,
      __ATOMIC_SEQ_CST);
#endif
  } while (!r);

  return ret;
}

static size_t refcounted_ptr_put(refcounted_ptr_t *p,
  void *ptr __attribute__((unused))) {

  refcounted_ptr_t old, new;
  size_t ret;
  bool r;

  do {

    /* Read the current state of the pointer. */
#ifdef __x86_64__
    /* It seems MOV on x86-64 is not guaranteed to be atomic on 128-bit
     * naturally aligned memory. The way to work around this is apparently the
     * following degenerate CMPXCHG.
     */
    old = __sync_val_compare_and_swap(p, *p, *p);
#else
    old = __atomic_load_n(p, __ATOMIC_SEQ_CST);
#endif
    struct refcounted_ptr p2;
    memcpy(&p2, &old, sizeof(old));

    /* Release our reference to it. */
    ASSERT(p2.ptr == ptr && "releasing a reference to a pointer after someone "
      "has changed the pointer source");
    ASSERT(p2.count > 0 && "releasing a reference to a pointer when it had no "
      "outstanding references");
    p2.count--;
    ret = p2.count;

    /* Try to commit our results. */
    memcpy(&new, &p2, sizeof(new));
#ifdef __x86_64__
    /* Make GCC >= 7.1 emit cmpxchg on x86-64. See
     * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80878.
     */
    r = __sync_bool_compare_and_swap(p, old, new);
#else
    r = __atomic_compare_exchange_n(p, &old, new, false, __ATOMIC_SEQ_CST,
      __ATOMIC_SEQ_CST);
#endif
  } while (!r);

  return ret;
}

static void refcounted_ptr_shift(refcounted_ptr_t *current, refcounted_ptr_t *next) {

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
 *                                                                             *
 * Expected usage is something like:                                           *
 *                                                                             *
 *   bool leader = rendezvous_arrive();                                        *
 *   if (leader) {                                                             *
 *     // single-threaded critical region                                      *
 *     ...                                                                     *
 *   }                                                                         *
 *   rendezvous_depart(leader);                                                *
 *                                                                             *
 ******************************************************************************/

static size_t rendezvous_pending = 1; // TODO: this should eventually be 'THREADS'
static semaphore_t rendezvous_barrier;

static void rendezvous_init(void) {
  if (semaphore_init(&rendezvous_barrier, 0) < 0) {
    perror("semaphore_init");
    exit(EXIT_FAILURE);
  }
}

/* Call this at the start of a rendezvous point.
 *
 * @return True if the caller was the last to arrive and henceforth dubbed the
 *   'leader'.
 */
static bool rendezvous_arrive(void) {

  /* Take a token from the rendezvous down-counter. */
  size_t id = __atomic_sub_fetch(&rendezvous_pending, 1, __ATOMIC_SEQ_CST);

  /* If we were the last to arrive then it was our arrival that dropped the
   * counter to zero.
   */
  return id == 0;
}

/* Call this at the end of a rendezvous point.
 *
 * @param leader Whether the caller is the 'leader'. If you call this when you
 *   are the 'leader' it will unblock all 'followers' at the rendezvous point.
 */
static void rendezvous_depart(bool leader) {
  if (leader) {

    /* Reset the counter for the next rendezvous. */
    rendezvous_pending += running_count;

    // TODO: This condition should become 'i < THREADS - 1'
    for (size_t i = 0; i < 0; i++) {
      int r __attribute__((unused)) = semaphore_post(&rendezvous_barrier);
      ASSERT(r == 0);
    }
  } else {
    int r __attribute__((unused)) = semaphore_wait(&rendezvous_barrier);
    ASSERT(r == 0);
  }
}

/* A trivial rendezvous (no critical section). */
static void rendezvous(void) {
  bool leader = rendezvous_arrive();
  rendezvous_depart(leader);
}

/* Remove the caller from the pool of threads who participate in this
 * rendezvous.
 */
static void rendezvous_thread_deinit(void) {

retry:;

  /* "Arrive" at the rendezvous to decrement the count of outstanding threads.
   */
  bool leader = rendezvous_arrive();

  if (leader) {
    /* There are two possible scenarios here:
     *
     *   1. We unfortunately opted out of this rendezvous while the remaining
     *      threads were arriving at one and we were the last to arrive.
     *   2. All other threads have already opted out of the rendezvous (begun
     *      exiting).
     */

    if (running_count > 0) {
      /* Case 1. Let's pretend we are also participating in the rendezvous and
       * just depart. This only works if the rendezvous point requires no
       * interstice action by the leader. Note that we need to bump the
       * rendezvous counter while we still have exclusive access to cause its
       * final state to be 'running + 1' when we retry.
       */
      rendezvous_pending++;
      rendezvous_depart(leader);

      /* Try to opt out again. */
      goto retry;
    }

    /* Case 2. We're fine, nothing to do. */
  }
}

/******************************************************************************/

/*******************************************************************************
 * 'Slots', an opaque wrapper around a state pointer                           *
 *                                                                             *
 * See usage of this in the state set below for its purpose.                   *
 ******************************************************************************/

typedef uintptr_t slot_t;

static slot_t slot_empty(void) {
  return 0;
}

static bool slot_is_empty(slot_t s) {
  return s == slot_empty();
}

static bool slot_is_tombstone(slot_t s) {
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
  size_t count;
};

/* Some utility functions for dealing with exponents. */

static size_t set_size(const struct set *set) {
  return ((size_t)1) << set->size_exponent;
}

static size_t set_index(const struct set *set, size_t index) {
  return index & (set_size(set) - 1);
}

/* The states we have encountered. This collection will only ever grow while
 * checking the model. Note that we have a global reference-counted pointer and
 * a local bare pointer. See below for an explanation.
 */
static refcounted_ptr_t global_seen;
static _Thread_local struct set *local_seen;

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
  int r __attribute__((unused)) = pthread_mutex_lock(&set_expand_mutex);
  ASSERT(r == 0);
}

static void set_expand_unlock(void) {
  int r __attribute__((unused)) = pthread_mutex_unlock(&set_expand_mutex);
  ASSERT(r == 0);
}


static void set_init(void) {

  int r = pthread_mutex_init(&set_expand_mutex, NULL);
  if (r < 0) {
    fprintf(stderr, "pthread_mutex_init failed: %s\n", strerror(r));
    exit(EXIT_FAILURE);
  }

  /* Allocate the set we'll store seen states in at some conservative initial
   * size.
   */
  struct set *set = xmalloc(sizeof(*set));
  set->size_exponent = INITIAL_SET_SIZE_EXPONENT;
  set->bucket = xcalloc(set_size(set), sizeof(set->bucket[0]));
  set->count = 0;

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

static void set_migrate(void) {

  trace(TC_SET, "assisting in set migration...");

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
       * everything in the old state is unique.
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
  size_t count = refcounted_ptr_put(&global_seen, local_seen);

  if (count == 0) {

    trace(TC_SET, "arrived at rendezvous point as leader");

    /* We were the last thread to release our reference to the old set. Clean it
     * up now. Note that we're using the pointer we just gave up our reference
     * count to, but we know no one else will be invalidating it.
     */
    free(local_seen->bucket);
    free(local_seen);

    /* Update the global pointer to the new set. We know all the above
     * migrations have completed and no one needs the old set.
     */
    refcounted_ptr_shift(&global_seen, &next_global_seen);
  }

  /* Now we need to make sure all the threads get to this point before any one
   * thread leaves. The purpose of this is to guarantee we only ever have at
   * most two seen sets "in flight". Without this rendezvous, one thread could
   * race ahead, fill the new set, and then decide to expand again while some
   * are still working on the old set. It's possible to make such a scheme work
   * but the synchronisation requirements just seem too complicated.
   */
  rendezvous();

  /* We're now ready to resume model checking. Note that we already have a
   * (reference counted) pointer to the now-current global seen set, so we don't
   * need to take a fresh reference to it.
   */
  local_seen = next;
}

static void set_expand(void) {

  set_expand_lock();

  /* Check if another thread beat us to expanding the set. */
  struct set *s = refcounted_ptr_get(&next_global_seen);
  (void)refcounted_ptr_put(&next_global_seen, s);
  if (s != NULL) {
    /* Someone else already expanded it. Join them in the migration effort. */
    set_expand_unlock();
    trace(TC_SET, "attempted expansion failed because another thread got there "
      "first");
    set_migrate();
    return;
  }

  trace(TC_SET, "expanding set from %zu slots to %zu slots...",
    (((size_t)1) << local_seen->size_exponent) / sizeof(slot_t),
    (((size_t)1) << (local_seen->size_exponent + 1)) / sizeof(slot_t));

  /* Create a set of double the size. */
  struct set *set = xmalloc(sizeof(*set));
  set->size_exponent = local_seen->size_exponent + 1;
  set->bucket = xcalloc(set_size(set), sizeof(set->bucket[0]));
  set->count = local_seen->count; /* will be true after migration */

  /* Advertise this as the newly expanded global set. */
  refcounted_ptr_set(&next_global_seen, set);

  /* We now need to migrate all slots from the old set to the new one, but we
   * can do this multithreaded.
   */
  next_migration = 0; /* initialise migration state */
  set_expand_unlock();
  set_migrate();
}

static bool set_insert(struct state *s, size_t *count) {

restart:;

  if (local_seen->count * 100 / set_size(local_seen) >= SET_EXPAND_THRESHOLD)
    set_expand();

  size_t index = set_index(local_seen, state_hash(s));

  size_t attempts = 0;
  for (size_t i = index; attempts < set_size(local_seen); i = set_index(local_seen, i + 1)) {

    /* Guess that the current slot is empty and try to insert here. */
    slot_t c = slot_empty();
    if (__atomic_compare_exchange_n(&local_seen->bucket[i], &c,
        state_to_slot(s), false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
      /* Success */
      *count = __atomic_add_fetch(&local_seen->count, 1, __ATOMIC_SEQ_CST);
      trace(TC_SET, "added state %p, set size is now %zu", s, *count);

      /* The maximum possible size of the seen state set should be constrained
       * by the number of possible states based on how many bits we are using to
       * represent the state data.
       */
      assert(STATE_SIZE_BITS > sizeof(size_t) * CHAR_BIT - 1 ||
        *count <= SIZE_C(1) << STATE_SIZE_BITS && "seen set size exceeds "
        "total possible number of states");

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
      trace(TC_SET, "skipped adding state %p that was already in set", s);
      return false;
    }

    attempts++;
  }

  /* If we reach here, the set is full. Expand it and retry the insertion. */
  set_expand();
  return set_insert(s, count);
}

/******************************************************************************/

static time_t START_TIME;

static unsigned long long gettime() {
  return (unsigned long long)(time(NULL) - START_TIME);
}

/* Prototypes for generated functions. */
static void init(void);
static _Noreturn void explore(void);

static int exit_with(int status) {

  /* Mark ourselves as no longer active. */
  running_count--;

  /* Opt out of the thread-wide rendezvous protocol. */
  rendezvous_thread_deinit();

  /* Make fired rule count visible globally. */
  rules_fired[thread_id] = rules_fired_local;

  if (thread_id == 0) {
    /* We are the initial thread. Wait on the others before exiting. */
    // TODO: the following condition shoud be 'i < sizeof(threads) / sizeof(thread[0])'
    for (size_t i = 0; i < 0; i++) {
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

    if (status == EXIT_SUCCESS) {
      printf("\n"
             "==========================================================================\n"
             "\n"
             "Status:\n"
             "\n"
             "\t%s%sNo error found.%s\n"
             "\n", green(), bold(), reset());
    }

    /* Calculate the total number of rules fired. */
    uintmax_t fire_count = 0;
    for (size_t i = 0; i < sizeof(rules_fired) / sizeof(rules_fired[0]); i++) {
      fire_count += rules_fired[i];
    }

    printf("State Space Explored:\n"
           "\n"
           "\t%zu states, %" PRIuMAX " rules fired in %llus.\n",
           local_seen->count, fire_count, gettime());

    exit(status);
  } else {
    pthread_exit((void*)(intptr_t)status);
  }
}

int main(void) {

  printf("Memory usage:\n"
         "\n"
         "\t* The size of each state is %zu bits (rounded up to %zu bytes).\n"
         "\t* The size of the hash table is %zu slots.\n"
         "\n",
         (size_t)STATE_SIZE_BITS, (size_t)STATE_SIZE_BYTES,
         ((size_t)1) << INITIAL_SET_SIZE_EXPONENT);

  START_TIME = time(NULL);

  if (COLOR == AUTO)
    istty = isatty(STDOUT_FILENO) != 0;

  rendezvous_init();

  set_init();
  queue_init();

  // TODO: Initialise after spawing new threads
  set_thread_init();

  init();

  printf("Progress Report:\n\n");

  explore();
}
