#ifndef __OPTIMIZE__
  #ifdef __clang__
    #ifdef __x86_64__
      #warning you are compiling without optimizations enabled. I would suggest -O3 -mcx16.
    #else
      #warning you are compiling without optimizations enabled. I would suggest -O3.
    #endif
  #else
    #ifdef __x86_64__
      #warning you are compiling without optimizations enabled. I would suggest -O3 -fwhole-program -mcx16.
    #else
      #warning you are compiling without optimizations enabled. I would suggest -O3 -fwhole-program.
    #endif
  #endif
#endif

/* Uncomment this to enable full checker trace output when debugging Rumur
 * itself on a small model.
 */
#if 0
  #define TRACE(fmt, args...)                                                  \
    eprint("%sTRACE%s: " fmt "\n",                                             \
      isatty(STDERR_FILENO) ? "\033[33m" : "",                                 \
      isatty(STDERR_FILENO) ? "\033[0m" : "" , ## args)
#else
  #define TRACE(args...) do { } while (0)
#endif

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
  #define SIZE_C(x) x ## ul
#endif

// TODO: define this dynamically
enum { BUCKET_COUNT = 1 << 20 };

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

/* Whether we've encountered an error or not. If a thread sees this set, they
 * should attempt to exit gracefully as soon as possible.
 */
static atomic_bool done;

/* GNU provides this under a different name. */
#ifndef PTHREAD_RECURSIVE_MUTEX_INITIALIZER
  #define PTHREAD_RECURSIVE_MUTEX_INITIALIZER PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
#endif

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

static const char *yellow() {
  if (COLOR == ON || (COLOR == AUTO && istty))
    return "\033[33m";
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
static pthread_mutex_t print_lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;

static __attribute__((format(printf, 1, 2))) void print(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int r __attribute__((unused)) = pthread_mutex_lock(&print_lock);
  assert(r == 0);
  (void)vprintf(fmt, ap);
  r = pthread_mutex_unlock(&print_lock);
  assert(r == 0);
  va_end(ap);
}

static __attribute__((format(printf, 1, 2))) void eprint(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int r __attribute__((unused)) = pthread_mutex_lock(&print_lock);
  assert(r == 0);
  (void)vfprintf(stderr, fmt, ap);
  r = pthread_mutex_unlock(&print_lock);
  assert(r == 0);
  va_end(ap);
}

/* The state of the current model. */
struct state {
  const struct state *previous;

  uint8_t data[STATE_SIZE_BYTES];
};

/* Print a counterexample trace terminating at the given state. */
static unsigned print_counterexample(const struct state *s);

static __attribute__((format(printf, 2, 3))) _Noreturn void error(
  const struct state *s, const char *fmt, ...) {

  bool expected = false;
  if (atomic_compare_exchange_strong(&done, &expected, true)) {

    va_list ap;
    va_start(ap, fmt);
    int r __attribute__((unused)) = pthread_mutex_lock(&print_lock);
    assert(r == 0);
    (void)vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);

    if (s != NULL) {
      fprintf(stderr, "Counterexample:\n");
      print_counterexample(s);
    }

    r = pthread_mutex_unlock(&print_lock);
    assert(r == 0);
  }
  exit(EXIT_FAILURE);
}

static void *xmalloc(size_t size) {
  void *p = malloc(size);
  if (p == NULL) {
    error(NULL, "out of memory");
  }
  return p;
}

static void *xcalloc(size_t count, size_t size) {
  void *p = calloc(count, size);
  if (p == NULL) {
    error(NULL, "out of memory");
  }
  return p;
}

static struct state *state_new(void) {
  return xcalloc(1, sizeof(struct state));
}

static int state_cmp(const struct state *a, const struct state *b) {
  return memcmp(a->data, b->data, sizeof(a->data));
}

// FIXME: do we need this function?
static __attribute__((unused)) bool state_eq(const struct state *a, const struct state *b) {
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

/* Print a state to stderr. This function is generated. */
static void state_print(const struct state *s);

static unsigned print_counterexample(const struct state *s) {

  if (s == NULL) {
    return 0;
  }

  /* Recurse so that we print the states in reverse-linked order, which
   * corresponds to the order in which they were traversed.
   */
  unsigned step = print_counterexample(s->previous) + 1;

  fprintf(stderr, " --- begin state %u ---\n", step);
  state_print(s);
  fprintf(stderr, " --- end state %u ---\n", step);
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

static __attribute__((unused)) value_t handle_read_raw(struct handle h) {
  static_assert(sizeof(unsigned __int128) > sizeof(value_t),
    "handle_read_raw() is implemented by reading data into a 128-bit scalar, "
    "potentially reading more than the width of a value. Value type is larger "
    "than 128 bits which prevents this.");

  if (h.width == 0) {
    TRACE("read value %" PRIVAL " from handle { %p, %zu, %zu }", (value_t)0,
      h.base, h.offset, h.width);
    return 0;
  }

  struct handle aligned = handle_align(h);
  unsigned __int128 v = handle_extract(aligned);
  v >>= h.offset % 8;
  v &= (((unsigned __int128)1) << h.width) - 1;

  value_t dest = (value_t)v;

  TRACE("read value %" PRIVAL " from handle { %p, %zu, %zu }", dest,
    h.base, h.offset, h.width);

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

  value_t dest = handle_read_raw(h);

  if (dest == 0) {
    error(s, "read of undefined value");
  }

  return decode_value(lb, ub, dest);
}

static __attribute__((unused)) void handle_write_raw(struct handle h,
    value_t value) {

  TRACE("writing value %" PRIVAL " to handle { %p, %zu, %zu }", value, h.base,
    h.offset, h.width);

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

  if (value < lb || value > ub || __builtin_sub_overflow(value, lb, &value) ||
      __builtin_add_overflow(value, 1, &value)) {
    error(s, "write of out-of-range value");
  }

  handle_write_raw(h, value);
}

static __attribute__((unused)) struct handle handle_narrow(struct handle h,
  size_t offset, size_t width) {

  ASSERT(h.offset + offset + width <= h.offset + h.width &&
    "narrowing a handle with values that actually expand it");

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

size_t queue_enqueue(struct state *s) {
  struct queue_node *n = malloc(sizeof(*n));
  if (n == NULL) {
    error(NULL, "out of memory");
  }
  n->s = s;

  int r __attribute__((unused)) = pthread_mutex_lock(&q.lock);
  ASSERT(r == 0);

  n->next = q.head;
  q.head = n;
  q.count++;

  TRACE("enqueued state %p, queue length is now %zu", s, q.count);

  size_t count = q.count;

  r = pthread_mutex_unlock(&q.lock);
  ASSERT(r == 0);

  return count;
}

struct state *queue_dequeue(void) {

  struct state *s = NULL;

  int r __attribute__((unused)) = pthread_mutex_lock(&q.lock);
  ASSERT(r == 0);

  struct queue_node *n = q.head;
  if (n != NULL) {
    q.head = n->next;
    q.count--;
    TRACE("dequeued state %p, queue length is now %zu", n->s, q.count);
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
    rendezvous_pending = 1; // TODO: should be 'THREADS'

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

enum { INITIAL_SET_SIZE = SET_CAPACITY / sizeof(struct state*) /
  (1 << (__builtin_ffs(STATE_SIZE_BYTES) +
    (__builtin_popcount(STATE_SIZE_BYTES) == 1 ? 0 : 1))) };

struct set {
  slot_t *bucket;
  size_t size;
  size_t count;
};

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

  if (pthread_mutex_init(&set_expand_mutex, NULL) < 0) {
    perror("pthread_mutex_init");
    exit(EXIT_FAILURE);
  }

  /* Allocate the set we'll store seen states in at some conservative initial
   * size.
   */
  struct set *set = xmalloc(sizeof(*set));
  set->size = INITIAL_SET_SIZE;
  set->bucket = xcalloc(set->size, sizeof(set->bucket[0]));

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
    if (start >= local_seen->size) {
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
        size_t index = state_hash(slot_to_state(s)) % next->size;
        for (size_t j = index; ; j = (j + 1) % next->size) {
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
    set_migrate();
    return;
  }

  /* Create a set of double the size. */
  struct set *set = xmalloc(sizeof(*set));
  set->size = local_seen->size * 2;
  set->bucket = xcalloc(set->size, sizeof(set->bucket[0]));
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

  if (local_seen->count * 100 / local_seen->size >= SET_EXPAND_THRESHOLD)
    set_expand();

  size_t index = state_hash(s) % local_seen->size;

  size_t attempts = 0;
  for (size_t i = index; attempts < local_seen->size; i = (i + 1) % local_seen->size) {

    /* Guess that the current slot is empty and try to insert here. */
    slot_t c = slot_empty();
    if (__atomic_compare_exchange_n(&local_seen->bucket[i], &c,
        state_to_slot(s), false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
      /* Success */
      *count = __atomic_add_fetch(&local_seen->count, 1, __ATOMIC_SEQ_CST);
      TRACE("added state %p, set size is now %zu", s, *count);
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
      TRACE("skipped adding state %p that was already in set", s);
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
static int explore(void);

int main(void) {

  print("State size: %zu bits\n", (size_t)STATE_SIZE_BITS);

  START_TIME = time(NULL);

  if (COLOR == AUTO)
    istty = isatty(STDOUT_FILENO) != 0;

  rendezvous_init();

  set_init();

  // TODO: Initialise after spawing new threads
  set_thread_init();

  init();
  int r = explore();

  print("%zu states covered%s\n", local_seen->count,
    r == EXIT_SUCCESS ? ", no errors found" : "");

  return r;
}
