#ifndef __OPTIMIZE__
  #ifdef __clang__
    #warning you are compiling without optimizations enabled. I would suggest -O3.
  #else
    #warning you are compiling without optimizations enabled. I would suggest -O3 -fwhole-program.
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

// ANSI colour code support.
// FIXME: thread safety

static int istty = -1;

static const char *green() {
  switch (COLOR) {
    case OFF:  return "";
    case ON:   return "\033[32m";
    case AUTO:
      if (istty == -1) {
        istty = isatty(STDOUT_FILENO);
      }
      return istty ? "\033[32m" : "";
  }
}

static const char *yellow() {
  switch (COLOR) {
    case OFF:  return "";
    case ON:   return "\033[33m";
    case AUTO:
      if (istty == -1) {
        istty = isatty(STDOUT_FILENO);
      }
      return istty ? "\033[33m" : "";
  }
}

static const char *reset() {
  switch (COLOR) {
    case OFF:  return "";
    case ON:   return "\033[0m";
    case AUTO:
      if (istty == -1) {
        istty = isatty(STDOUT_FILENO);
      }
      return istty ? "\033[0m" : "";
  }
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

  ASSERT(h.offset + offset + width <= h.offset + width &&
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
 * State set                                                                   *
 *                                                                             *
 * The following implementation provides a set for storing the seen states.    *
 * There is no support for testing whether something is in the set or for      *
 * removing elements, only thread-safe insertion of elements.                  *
 ******************************************************************************/

enum { INITIAL_SET_SIZE = SET_CAPACITY / sizeof(struct state*) /
  (1 << (__builtin_ffs(STATE_SIZE_BYTES) +
    (__builtin_popcount(STATE_SIZE_BYTES) == 1 ? 0 : 1))) };

/* The states we have encountered. This collection will only ever grow while
 * checking the model.
 */
static struct {
  struct state **bucket;
  size_t size;
  size_t count;
} seen;

static void set_init(void) {
  seen.size = INITIAL_SET_SIZE;
  seen.bucket = xcalloc(seen.size, sizeof(seen.bucket[0]));
}

// TODO: parallelise/thread-safe this
static void set_expand(void) {

  /* Create a set of double the size. */
  size_t new_size = seen.size * 2;
  struct state **bucket = xcalloc(new_size, sizeof(bucket[0]));

  /* Migrate all elements. */
  for (size_t i = 0; i < seen.size; i++) {
    if (seen.bucket[i] != NULL) {
      size_t index = state_hash(seen.bucket[i]) % new_size;
      for (size_t j = index; ; j = (j + 1) % new_size) {
        if (bucket[j] == NULL) {
          bucket[j] = seen.bucket[i];
          break;
        }
      }
    }
  }

  /* Free the old set and install the new one. */
  free(seen.bucket);
  seen.bucket = bucket;
  seen.size = new_size;
}

static bool set_insert(struct state *s, size_t *count) {

  if (seen.count * 100 / seen.size >= SET_EXPAND_THRESHOLD)
    set_expand();

  size_t index = state_hash(s) % seen.size;

  size_t attempts = 0;
  for (size_t i = index; attempts < seen.size; i = (i + 1) % seen.size) {

retry:;
    struct state *c = __atomic_load_n(&seen.bucket[i], __ATOMIC_SEQ_CST);

    /* If this slot is empty, try to insert it here. */
    if (c == NULL) {
      if (!__atomic_compare_exchange(&seen.bucket[i], &c, &s, false,
          __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
        /* Failed */
        goto retry;
      }
      *count = __atomic_add_fetch(&seen.count, 1, __ATOMIC_SEQ_CST);
      TRACE("added state %p, set size is now %zu", s, *count);
      return true;
    }

    /* If we find this already in the set, we're done. */
    if (state_eq(s, c)) {
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

  set_init();

  init();
  int r = explore();

  print("%zu states covered%s\n", seen.count,
    r == EXIT_SUCCESS ? ", no errors found" : "");

  return r;
}
