#ifndef __OPTIMIZE__
  #ifdef __clang__
    #warning you are compiling without optimizations enabled. I would suggest -O3.
  #else
    #warning you are compiling without optimizations enabled. I would suggest -O3 -fwhole-program.
  #endif
#endif

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <ctime>
#include <cinttypes>
#include <climits>
#include <condition_variable>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <limits>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unistd.h>
#include <unordered_set>
#include <utility>
#include <vector>

#ifdef USE_TBB
  #include <tbb/concurrent_unordered_set.h>
#endif

/* The size of the compressed state data in bytes. */
static constexpr size_t STATE_SIZE_BYTES = STATE_SIZE_BITS / 8 + (STATE_SIZE_BITS % 8 == 0 ? 0 : 1);

namespace { class Semaphore {

 private:
  long value = 0;
  std::condition_variable cv;
  std::mutex lock;

 public:
  void post(unsigned count = 1) {
    std::unique_lock<std::mutex> lk(lock);
    while (value < 0 && count > 0) {
      value++;
      count--;
      lk.unlock();
      cv.notify_one();
      lk.lock();
    }
    value += count;
  }

  void wait() {
    std::unique_lock<std::mutex> lk(lock);
    value--;
    if (value < 0) {
      cv.wait(lk);
    }
  }
}; }

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

/* Define our own recursive mutex that collapses to no-ops if we're not running
 * multithreaded.
 */

namespace {
template<unsigned long THREAD_COUNT, bool NEEDS_MUTEX = THREAD_COUNT != 1>
class RecursiveMutex;
}

namespace {
template<unsigned long THREAD_COUNT>
class RecursiveMutex<THREAD_COUNT, false> {

 public:
  void lock() { };
  void unlock() { };
};
}

namespace {
template<unsigned long THREAD_COUNT>
class RecursiveMutex<THREAD_COUNT, true> {

 private:
  std::recursive_mutex mutex;

 public:
  void lock() {
    mutex.lock();
  }

  void unlock() {
    mutex.unlock();
  }
};
}

/* A lock that should be held whenever printing to stdout or stderr. This is a
 * way to prevent the output of one thread being interleaved with the output of
 * another.
 */
static RecursiveMutex<THREADS> print_lock;

[[gnu::format(printf, 1, 2)]] static void print(const char *fmt, ...) {
  std::lock_guard<decltype(print_lock)> lock(print_lock);
  va_list ap;
  va_start(ap, fmt);
  (void)vprintf(fmt, ap);
  va_end(ap);
}

[[gnu::format(printf, 2, 3)]] static void fprint(FILE *f, const char *fmt, ...) {
  std::lock_guard<decltype(print_lock)> lock(print_lock);
  va_list ap;
  va_start(ap, fmt);
  (void)vfprintf(f, fmt, ap);
  va_end(ap);
}

/* An exception that is thrown that is not related to a specific current state.
 * This is used within infrastructure code.
 */
namespace {
class Error : public std::runtime_error {

 public:
  using std::runtime_error::runtime_error;

};
}

namespace {
template<typename T>
struct Allocator {

 private:
  T *cached = nullptr;

 public:
  T *alloc() {
    if (cached != nullptr) {
      T *t = cached;
      cached = nullptr;
      return t;
    }
    T *t = reinterpret_cast<T*>(new unsigned char[sizeof(T)]);
    return t;
  }

  void free(T *t) {
    ASSERT(cached == nullptr);
    cached = t;
  }

  ~Allocator() {
    delete[] reinterpret_cast<unsigned char*>(cached);
  }
};
}

namespace {
struct State {

 public:
  static const State *ORIGIN;

  std::array<uint8_t, STATE_SIZE_BYTES> data;
  const State *previous = nullptr;

 public:
  State() = default;
  State(const State&) = default;
  State(State&&) = default;
  State &operator=(const State&) = default;
  State &operator=(State&&) = default;

  State *duplicate(Allocator<State> &a) const {
    return new(a.alloc()) State(this);
  }

  bool operator==(const State &other) const {
    return data == other.data;
  }

  bool operator!=(const State &other) const {
    return !(*this == other);
  }

  // FIXME: the following assume little endian

  int64_t read(size_t offset, size_t width) const {
    ASSERT(width <= sizeof(int64_t) * CHAR_BIT && "read of too large value");
    ASSERT(offset <= data.size() * 8 - 1 && "out of bounds read");
    unsigned __int128 v = 0;
    size_t window = width / 8 + ((offset + width) % 8 == 0 ? 0 : 1);
    memcpy(&v, static_cast<const uint8_t*>(data.data()) + (offset / 8), window);
    v >>= offset % 8;
    v &= (static_cast<unsigned __int128>(1) << width) - 1;
    return static_cast<int64_t>(v);
  }

  void write(size_t offset, size_t width, int64_t value) {
    ASSERT(width <= sizeof(int64_t) * CHAR_BIT && "write of too large a value");
    ASSERT(width == 64 || (UINT64_C(1) << width > uint64_t(value) && "write of too large a value"));
    unsigned __int128 v = 0;
    size_t window = width / 8 + ((offset + width) % 8 == 0 ? 0 : 1);
    memcpy(&v, static_cast<const uint8_t*>(data.data()) + (offset / 8), window);
    v = (v & ~(((static_cast<unsigned __int128>(1) << width) - 1) << (offset % 8)))
      | (static_cast<unsigned __int128>(value) << (offset % 8));
    memcpy(static_cast<uint8_t*>(data.data()) + (offset / 8), &v, window);
  }

  size_t hash() const {
    // FIXME: a proper hash function that works for larger SIZE_BITS
    static_assert(STATE_SIZE_BITS <= sizeof(size_t) * CHAR_BIT, "FIXME");
    size_t s = 0;
    memcpy(&s, data.data(), data.size());
    return s;
  }

  static size_t width() {
    return STATE_SIZE_BITS;
  }

 private:
  State(const State *s): data(s->data), previous(s) { }
};

const State *State::ORIGIN = reinterpret_cast<const State*>(-1);
}

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

/* A queue of states that can be either thread-safe or not depending on whether
 * we're running multithreaded.
 */

namespace {
template<bool NEEDS_MUTEX = THREADS != 1>
class Queue;
}

namespace {
template<>
class Queue<false> {

 private:
  std::queue<State*> q;

 public:
  size_t push(State *t, unsigned long) {
    q.push(t);
    return q.size();
  }

  State *pop(unsigned long&) {
    if (q.empty()) {
      return nullptr;
    }
    State *t = q.front();
    q.pop();
    return t;
  }

  size_t size() const {
    return q.size();
  }
};
}

namespace {
template<>
class Queue<true> {

 private:
  std::array<std::mutex, THREADS> lock;
  std::array<std::queue<State*>, THREADS> q;

 public:
  size_t push(State *t, unsigned long queue_id) {
    ASSERT(queue_id < q.size());
    std::lock_guard<std::mutex> l(lock[queue_id]);
    q[queue_id].push(t);
    return q[queue_id].size();
  }

  State *pop(unsigned long &queue_id) {
    ASSERT(queue_id < q.size());
    for (size_t i = 0; i < q.size(); i++) {
      {
        std::lock_guard<std::mutex> l(lock[queue_id]);
        if (!q[queue_id].empty()) {
          State *t = q[queue_id].front();
          q[queue_id].pop();
          return t;
        }
      }
    }
    return nullptr;
  }

  size_t size() const {
    size_t s = 0;
    for (size_t i = 0; i < q.size(); i++) {
      s += q[i].size();
    }
    return s;
  }
};
}

/* A set of states with either thread-safe or -unsafe methods based on how many
 * threads we're using.
 */

namespace {
template<bool DYNAMIC = SET_CAPACITY == 0, bool NEEDS_MUTEX = THREADS != 1>
class Set;
}

namespace {
template<>
class Set<true, false> {

 private:
  std::unordered_set<State*, state_hash, state_eq> s;

 public:
  std::tuple<size_t, bool, State*> insert(State *t) {
    auto r = s.insert(t);
    return std::tuple<size_t, bool, State*>(size(), r.second, t);
  }

  size_t size() const {
    return s.size();
  }
};
}

namespace {
#ifdef USE_TBB
template<>
class Set<true, true> {

 private:
  tbb::concurrent_unordered_set<State*, state_hash, state_eq> s;

 public:
  std::tuple<size_t, bool, State*> insert(State *t) {
    auto r = s.insert(t);
    return std::tuple<size_t, bool, State*>(size(), r.second, t);
  }

  size_t size() const {
    return s.size();
  }
};
#else
template<>
class Set<true, true> : Set<true, false> {

 private:
  mutable std::mutex lock;

 public:
  std::tuple<size_t, bool, State*> insert(State *t) {
    std::lock_guard<decltype(lock)> l(lock);
    return Set<true, false>::insert(t);
  }

  size_t size() const {
    std::lock_guard<decltype(lock)> l(lock);
    return Set<true, false>::size();
  }
};
#endif
}

namespace {
template<>
class Set<false, false> {

 public:
  enum { COUNT = SET_CAPACITY / sizeof(State) };

 private:
  State *s;
  size_t used = 0;

 public:
  Set(): s(static_cast<State*>(calloc(COUNT, sizeof(State)))) {
    if (s == nullptr) {
      throw Error("failed to allocate closed hash set");
    }
  }

  std::tuple<size_t, bool, State*> insert(State *t) {
    if (COUNT != 0) {
      size_t slot = state_hash()(t) % COUNT;
      for (size_t i = 0; i < COUNT; i++) {
        if (is_empty(slot)) {
          {
            /* If State has a vtable, the entries of our array 's' will not have
             * their vtables initialised (as they are zeroed). Hence the
             * assignment we do below will leave us with a malformed State in
             * 's[slot]'. I.e. an object with the correct data but a vtable full
             * of null pointers.
             *
             * A sensible compiler should not emit a vtable for State but
             * perhaps the user's compiler does something foolish or perhaps a
             * careless refactoring of the resource files adds a vtable to
             * State. In either event we'd like to catch this problem instead of
             * leaving the user to confront a confusing segfault at runtime.
             *
             * The following is a hacky check for whether State has a vtable. By
             * comparing its size of an equivalent object that is certainly a
             * POD we should be able to detect if the compiler laid out the two
             * differently.
             */
            struct pod {
              std::array<uint8_t, STATE_SIZE_BYTES> data;
              State *previous;
            };
            static_assert(sizeof(pod) == sizeof(State), "State has a vtable");
          }
          s[slot] = *t;
          used++;
          return std::tuple<size_t, bool, State*>(used, true, &s[slot]);
        } else if (state_eq()(t, &s[slot])) {
          return std::tuple<size_t, bool, State*>(used, false, t);
        }
        slot = (slot + 1) % COUNT;
      }
    }
    throw Error("closed hash set full");
  }

  size_t size() const {
    return used;
  }

  ~Set() {
    free(s);
  }

 private:
  bool is_empty(size_t slot) const {
    ASSERT(slot < COUNT);
    return s[slot].previous == nullptr;
  }
};
}

namespace {
template<>
class Set<false, true> : Set<false, false> {

 private:
  mutable std::mutex lock;

 public:
  std::tuple<size_t, bool, State*> insert(State *t) {
    std::lock_guard<decltype(lock)> l(lock);
    return Set<false, false>::insert(t);
  }

  size_t size() const {
    std::lock_guard<decltype(lock)> l(lock);
    return Set<false, false>::size();
  }
};
}

namespace {
struct StartState {

 public:
  const std::string name;
  const std::function<void(State&)> body;

 public:
  StartState(const std::string &name_, std::function<void(State&)> body_):
    name(name_), body(body_) { }
};
}

namespace {
struct Invariant {

 public:
  const std::string name;
  const std::function<bool(const State&)> guard;

 public:
  Invariant(const std::string &name_, std::function<bool(const State&)> guard_):
    name(name_), guard(guard_) { }
};
}

namespace {
struct Rule {

 public:
  const std::string name;
  const std::function<bool(const State&)> guard;
  const std::function<void(State&)> body;

 public:
  Rule(const std::string &name_, std::function<bool(const State&)> guard_,
    std::function<void(State&)> body_):
    name(name_), guard(guard_), body(body_) { }

 private:
  class iterator {
   private:
    const Rule &rule;
    State &origin;
    Allocator<State> *allocator;
    bool end;

   public:
    iterator(const Rule &rule_, State &origin_, Allocator<State> &allocator_, bool end_ = false):
      rule(rule_), origin(origin_), allocator(&allocator_), end(end_) {
      if (!end && !rule.guard(origin)) {
        ++*this;
      }
    }

    iterator &operator++() {
      if (!end) {
        end = true;
      }
      return *this;
    }

    bool operator==(const iterator &other) const {
      // Note that we deliberately compare by pointer here.
      return &rule == &other.rule && &origin == &other.origin && end == other.end;
    }

    bool operator!=(const iterator &other) const {
      return !(*this == other);
    }

    State *operator*() const {
      ASSERT(!end);
      State *d = origin.duplicate(*allocator);
      rule.body(*d);
      return d;
    }
  };

  class iterable {
   private:
    const Rule &rule;
    State &origin;
    Allocator<State> *allocator;

   public:
    iterable(const Rule &rule_, State &origin_, Allocator<State> &allocator_):
      rule(rule_), origin(origin_), allocator(&allocator_) { }

    iterator begin() const {
      return iterator(rule, origin, *allocator);
    }

    iterator end() const {
      return iterator(rule, origin, *allocator, true);
    }
  };

 public:
  iterable get_iterable(State &origin, Allocator<State> &allocator) const {
    return iterable(*this, origin, allocator);
  }
};
}

/* Overflow-safe helpers for doing 64-bit arithmetic. The compiler built-ins
 * used are implemented in modern GCC and Clang. If you're using another
 * compiler, you'll have to implement these yourself.
 */

[[gnu::unused]] static int64_t add(int64_t a, int64_t b) {
  int64_t r;
  if (OVERFLOW_CHECKS_ENABLED) {
    if (__builtin_add_overflow(a, b, &r)) {
      throw Error("integer overflow in addition");
    }
  } else {
    r = a + b;
  }
  return r;
}

[[gnu::unused]] static int64_t sub(int64_t a, int64_t b) {
  int64_t r;
  if (OVERFLOW_CHECKS_ENABLED) {
    if (__builtin_sub_overflow(a, b, &r)) {
      throw Error("integer overflow in subtraction");
    }
  } else {
    r = a - b;
  }
  return r;
}

[[gnu::unused]] static int64_t mul(int64_t a, int64_t b) {
  int64_t r;
  if (OVERFLOW_CHECKS_ENABLED) {
    if (__builtin_mul_overflow(a, b, &r)) {
      throw Error("integer overflow in multiplication");
    }
  } else {
    r = a * b;
  }
  return r;
}

[[gnu::unused]] static int64_t divide(int64_t a, int64_t b) {
  if (b == 0) {
    throw Error("division by zero");
  }

  if (OVERFLOW_CHECKS_ENABLED) {
    if (a == std::numeric_limits<int64_t>::min() && b == -1) {
      throw Error("integer overflow in division");
    }
  }
  return a / b;
}

[[gnu::unused]] static int64_t mod(int64_t a, int64_t b) {
  if (b == 0) {
    throw Error("modulus by zero");
  }

  // Is INT64_MIN % -1 UD? Reading the C spec I'm not sure.
  if (OVERFLOW_CHECKS_ENABLED) {
    if (a == std::numeric_limits<int64_t>::min() && b == -1) {
      throw Error("integer overflow in modulo");
    }
  }
  return a % b;
}

[[gnu::unused]] static int64_t negate(int64_t a) {
  if (OVERFLOW_CHECKS_ENABLED) {
    if (a == std::numeric_limits<int64_t>::min()) {
      throw Error("integer overflow in negation");
    }
  }
  return -a;
}

namespace {
struct Number {

 public:
  int64_t value;

 public:
  Number() = delete;
  constexpr Number(int64_t value_): value(value_) { }
  Number(const Number&) = default;
  Number(Number&&) = default;
  Number &operator=(const Number&) = delete;
  Number &operator=(Number&&) = delete;

  Number operator+(const Number &other) const {
    return add(value, other.value);
  }

  Number operator-(const Number &other) const {
    return sub(value, other.value);
  }

  Number operator*(const Number &other) const {
    return mul(value, other.value);
  }

  Number operator/(const Number &other) const {
    return divide(value, other.value);
  }

  Number operator%(const Number &other) const {
    return mod(value, other.value);
  }

  bool operator<(const Number &other) const {
    return value < other.value;
  }

  bool operator>(const Number &other) const {
    return value > other.value;
  }

  bool operator==(const Number &other) const {
    return value == other.value;
  }

  bool operator!=(const Number &other) const {
    return value != other.value;
  }

  bool operator<=(const Number &other) const {
    return value <= other.value;
  }

  bool operator>=(const Number &other) const {
    return value >= other.value;
  }

  void print(FILE *f, const char *title) const {
    fprint(f, "%s = %" PRId64, title, value);
  }
};
}

/* Compile-time discrimination as to whether a type is Number. */
namespace {
template<typename>
struct isaNumber : public std::false_type { };
}

namespace {
template<>
struct isaNumber<Number> : public std::true_type { };
}

namespace {
template<int64_t MIN, int64_t MAX>
struct RangeReference;
}

namespace {
template<int64_t MIN, int64_t MAX>
struct RangeValue;
}

namespace {
template<int64_t MIN, int64_t MAX>
struct Range {

 public:
  using reference_type = RangeReference<MIN, MAX>;
  using value_type = RangeValue<MIN, MAX>;

 public:
  Range &operator=(const Range &other) {
    set_value(other.get_value());
    return *this;
  }

  Range &operator=(const Number &other) {
    set_value(other.value);
    return *this;
  }

  static RangeValue<MIN, MAX> make() {
    return RangeValue<MIN, MAX>(MIN);
  }

  static RangeReference<MIN, MAX> make(State &container, size_t offset) {
    return RangeReference<MIN, MAX>(container, offset);
  }

  static const RangeReference<MIN, MAX> make(const State &container, size_t offset) {
    return RangeReference<MIN, MAX>(const_cast<State&>(container), offset);
  }

  size_t zero_based_value() const {
    size_t r;
    if (__builtin_sub_overflow(get_value(), MIN, &r))
      throw Error("overflow in calculating index value");
    return r;
  }

  RangeValue<MIN, MAX> operator+(const Range &other) const {
    return add(get_value(), other.get_value());
  }

  RangeValue<MIN, MAX> operator+(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of the range");
    }
    int64_t v = add(get_value(), other.value);
    if (v < MIN || v > MAX) {
      throw Error("result of addition is out of range");
    }
    return v;
  }

  RangeValue<MIN, MAX> operator-(const Range &other) const {
    return sub(get_value(), other.get_value());
  }

  RangeValue<MIN, MAX> operator-(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of the range");
    }
    int64_t v = sub(get_value(), other.value);
    if (v < MIN || v > MAX) {
      throw Error("result of subtraction is out of range");
    }
    return v;
  }

  RangeValue<MIN, MAX> operator*(const Range &other) const {
    return mul(get_value(), other.get_value());
  }

  RangeValue<MIN, MAX> operator*(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in multiplication");
    }
    int64_t v = mul(get_value(), other.value);
    if (v < MIN || v > MAX) {
      throw Error("result of multiplication is out of range");
    }
    return v;
  }

  RangeValue<MIN, MAX> operator/(const Range &other) const {
    return divide(get_value(), other.get_value());
  }

  RangeValue<MIN, MAX> operator/(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in division");
    }
    int64_t v = divide(get_value(), other.value);
    if (v < MIN || v > MAX) {
      throw Error("result of division is out of range");
    }
    return v;
  }

  RangeValue<MIN, MAX> operator%(const Range &other) const {
    return mod(get_value(), other.get_value());
  }

  RangeValue<MIN, MAX> operator%(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in mod");
    }
    int64_t v = mod(get_value(), other.value);
    if (v < MIN || v > MAX) {
      throw Error("result of mod is out of range");
    }
    return v;
  }

  bool operator<(const Range &other) const {
    return get_value() < other.get_value();
  }

  bool operator<(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in <");
    }
    return get_value() < other.value;
  }

  bool operator>(const Range &other) const {
    return get_value() > other.get_value();
  }

  bool operator>(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in >");
    }
    return get_value() > other.value;
  }

  bool operator==(const Range &other) const {
    return get_value() == other.get_value();
  }

  bool operator==(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in ==");
    }
    return get_value() == other.value;
  }

  bool operator!=(const Range &other) const {
    return get_value() != other.get_value();
  }

  bool operator!=(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in !=");
    }
    return get_value() != other.value;
  }

  bool operator<=(const Range &other) const {
    return get_value() <= other.get_value();
  }

  bool operator<=(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in <=");
    }
    return get_value() <= other.value;
  }

  bool operator>=(const Range &other) const {
    return get_value() >= other.get_value();
  }

  bool operator>=(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in >=");
    }
    return get_value() >= other.value;
  }

  virtual int64_t get_value() const = 0;

  virtual void set_value(int64_t v) = 0;

  void print(FILE *f, const char *title) const {
    fprint(f, "%s = %" PRId64, title, get_value());
  }

  static constexpr size_t count() {
    return MAX - MIN + 1;
  }

  static constexpr size_t width() {
    return MAX - MIN == 0 ? 0 : sizeof(unsigned long long) * CHAR_BIT - __builtin_clzll(MAX - MIN);
  }

  static constexpr int64_t min() {
    return MIN;
  }

  static constexpr int64_t max() {
    return MAX;
  }

#if 0

 private:
  class iterator {

   private:
    int64_t value;

   public:
    // TODO: support a step value in the ctor?
    iterator(int64_t value_): value(value_) { }

    iterator &operator++() {
      value++;
      return *this;
    }

    bool operator==(const iterator &other) const {
      return value == other.value;
    }

    bool operator!=(const iterator &other) const {
      return !(*this == other);
    }

    RangeBase operator*() const {
      return RangeBase(value);
    }
  };

 public:
  iterator begin() const {
    return iterator(MIN);
  }

  iterator end() const {
    return iterator(MAX + 1);
  }
#endif
};
}

template<int64_t MIN, int64_t MAX>
static RangeValue<MIN, MAX> operator+(const Number &a, const Range<MIN, MAX> &b) {
  return b + a;
}

template<int64_t MIN, int64_t MAX>
static RangeValue<MIN, MAX> operator-(const Number &a, const Range<MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range for subtraction");
  }
  int64_t v = sub(a.value, b.get_value());
  if (v < MIN || v > MAX) {
    throw Error("result of subtraction is out of range");
  }
  return v;
}

template<int64_t MIN, int64_t MAX>
static RangeValue<MIN, MAX> operator*(const Number &a, const Range<MIN, MAX> &b) {
  return b * a;
}

template<int64_t MIN, int64_t MAX>
static RangeValue<MIN, MAX> operator/(const Number &a, const Range<MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in division");
  }
  int64_t v = divide(a.value, b.get_value());
  if (v < MIN || v > MAX) {
    throw Error("result of division is out of range");
  }
  return v;
}

template<int64_t MIN, int64_t MAX>
static RangeValue<MIN, MAX> operator%(const Number &a, const Range<MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in mod");
  }
  int64_t v = mod(a.value, b.get_value());
  if (v < MIN || v > MAX) {
    throw Error("result of mod is out of range");
  }
  return v;
}

template<int64_t MIN, int64_t MAX>
static bool operator<(const Number &a, const Range<MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in <");
  }
  return a.value < b.get_value();
}

template<int64_t MIN, int64_t MAX>
static bool operator>(const Number &a, const Range<MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in >");
  }
  return a.value > b.get_value();
}

template<int64_t MIN, int64_t MAX>
static bool operator==(const Number &a, const Range<MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in ==");
  }
  return a.value == b.get_value();
}

template<int64_t MIN, int64_t MAX>
static bool operator!=(const Number &a, const Range<MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in !=");
  }
  return a.value != b.get_value();
}

template<int64_t MIN, int64_t MAX>
static bool operator<=(const Number &a, const Range<MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in <=");
  }
  return a.value <= b.get_value();
}

template<int64_t MIN, int64_t MAX>
static bool operator>=(const Number &a, const Range<MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in >=");
  }
  return a.value >= b.get_value();
}

namespace {
template<typename>
struct isaRange : public std::false_type { };
}

namespace {
template<int64_t MIN, int64_t MAX>
struct isaRange<Range<MIN, MAX>> : public std::true_type { };
}

namespace {
template<int64_t MIN, int64_t MAX>
struct RangeReference : public Range<MIN, MAX> {

 public:
  State *container;
  const size_t offset;

 public:
  RangeReference() = delete;
  RangeReference(State &container_, size_t offset_):
    container(&container_), offset(offset_) { }
  RangeReference(const RangeReference&) = default;
  RangeReference(RangeReference&&) = default;

  using Range<MIN, MAX>::operator=;

  RangeReference &operator=(const RangeReference &other) {
    return static_cast<RangeReference&>(Range<MIN, MAX>::operator=(other));
  }

  int64_t get_value() const final {
    ASSERT(container != nullptr);
    return container->read(offset, width()) + MIN;
  }

  void set_value(int64_t v) final {
    ASSERT(container != nullptr);
    container->write(offset, width(), v - MIN);
  }

  static constexpr size_t width() {
    return Range<MIN, MAX>::width();
  }
};
}

namespace {
template<int64_t MIN, int64_t MAX>
struct RangeValue : public Range<MIN, MAX> {

 public:
  int64_t value;

 public:
  RangeValue() = delete;
  RangeValue(int64_t value_): value(value_) { }
  RangeValue(const RangeValue&) = default;
  RangeValue(RangeValue&&) = default;

  using Range<MIN, MAX>::operator=;

  RangeValue &operator=(const RangeValue &other) {
    return static_cast<RangeValue&>(Range<MIN, MAX>::operator=(other));
  }

  int64_t get_value() const final {
    return value;
  }

  void set_value(int64_t v) final {
    value = v;
  }
};
}

namespace {
template<typename INDEX_T, typename ELEMENT_T>
class ArrayReference;
}

namespace {
template<typename INDEX_T, typename ELEMENT_T>
class ArrayValue;
}

namespace {
template<typename INDEX_T, typename ELEMENT_T>
class Array {

 public:
  static const ArrayReference<INDEX_T, ELEMENT_T> make(const State &container, size_t offset) {
    return ArrayReference<INDEX_T, ELEMENT_T>(const_cast<State&>(container), offset);
  }

  static ArrayReference<INDEX_T, ELEMENT_T> make(State &container, size_t offset) {
    return ArrayReference<INDEX_T, ELEMENT_T>(container, offset);
  }

  /* operator[] that takes a Number and is only valid if our index type is a
   * range.
   */
  template<typename T = INDEX_T, typename std::enable_if<isaRange<T>::value>::type* = nullptr>
  typename ELEMENT_T::reference_type operator[](const Number &index) {
    if (index.value < INDEX_T::min() || index.value > INDEX_T::max()) {
      throw Error("out of range access to array element " + std::to_string(index.value));
    }
    return get(index.value - INDEX_T::min());
  }

  template<typename T = INDEX_T, typename std::enable_if<isaRange<T>::value>::type* = nullptr>
  const typename ELEMENT_T::reference_type operator[](const Number &index) const {
    if (index.value < INDEX_T::min() || index.value > INDEX_T::max()) {
      throw Error("out of range access to array element " + std::to_string(index.value));
    }
    return get(index.value - INDEX_T::min());
  }

  typename ELEMENT_T::reference_type operator[](const INDEX_T &index) {
    return get(index.zero_based_value());
  }

  const typename ELEMENT_T::reference_type operator[](const INDEX_T &index) const {
    return get(index.zero_based_value());
  }

 private:
  virtual typename ELEMENT_T::reference_type get(size_t index) = 0;
  virtual const typename ELEMENT_T::reference_type get(size_t index) const = 0;

 public:
  void print(FILE*, const char*) const {
    // TODO: We want something like a range-based for loop over the index type
    // calling print() on the element type
  }

  static constexpr size_t count() {
    return INDEX_T::count() * ELEMENT_T::count();
  }

  static constexpr size_t width() {
    return INDEX_T::count() * ELEMENT_T::width();
  }
};
}

namespace {
template<typename INDEX_T, typename ELEMENT_T>
class ArrayReference : public Array<INDEX_T, ELEMENT_T> {

 public:
  State *container;
  const size_t offset;

 public:
  ArrayReference() = delete;
  ArrayReference(State &container_, size_t offset_):
    container(&container_), offset(offset_) { }
  ArrayReference(const ArrayReference&) = default;
  ArrayReference(ArrayReference&&) = default;

  /* FIXME: For some reason the enable_if'd operator[] definitions don't get
   * inherited and we need to repeat them to make them accessible. Is this
   * related to them not depending on the class template parameters?
   */
 public:
  template<typename T = INDEX_T, typename std::enable_if<isaRange<T>::value>::type* = nullptr>
  typename ELEMENT_T::reference_type operator[](const Number &index) {
    return static_cast<Array<INDEX_T, ELEMENT_T>&>(*this)[index];
  }

  template<typename T = INDEX_T, typename std::enable_if<isaRange<T>::value>::type* = nullptr>
  const typename ELEMENT_T::reference_type operator[](const Number &index) const {
    return static_cast<const Array<INDEX_T, ELEMENT_T>&>(*this)[index];
  }

  using Array<INDEX_T, ELEMENT_T>::operator[];

 private:
  typename ELEMENT_T::reference_type get(size_t index) final {
    return typename ELEMENT_T::reference_type(*container, offset + index * ELEMENT_T::width());
  }

  const typename ELEMENT_T::reference_type get(size_t index) const final {
    return typename ELEMENT_T::reference_type(*container, offset + index * ELEMENT_T::width());
  }
};
}

namespace {
template<typename INDEX_T, typename ELEMENT_T>
class ArrayValue : public Array<INDEX_T, ELEMENT_T> {

 public:
  std::array<typename ELEMENT_T::value_type, INDEX_T::count()> value;

 public:
  template<typename T = INDEX_T, typename std::enable_if<isaRange<T>::value>::type* = nullptr>
  typename ELEMENT_T::reference_type operator[](const Number &index) {
    return static_cast<Array<INDEX_T, ELEMENT_T>&>(*this)[index];
  }

  template<typename T = INDEX_T, typename std::enable_if<isaRange<T>::value>::type* = nullptr>
  const typename ELEMENT_T::reference_type operator[](const Number &index) const {
    return static_cast<const Array<INDEX_T, ELEMENT_T>&>(*this)[index];
  }

  using Array<INDEX_T, ELEMENT_T>::operator[];

 private:
  typename ELEMENT_T::value_type &get(size_t index) final {
    return value[index];
  }

  const typename ELEMENT_T::value_type get(size_t index) const final {
    return value[index];
  }
};
}

namespace {
template<typename = void>
class BooleanReference;
}

namespace {
template<typename = void>
class BooleanValue;
}

/* XXX: We need to use a dummy template parameter to allow us to use the (as
 * yet) undefined classes BooleanReference and BooleanValue in Boolean::make. By
 * the time we instantiate this template, the compiler will have seen
 * definitions of both. Perhaps there is a nicer way to achieve this?
 */
namespace {
template<typename T = void>
class Boolean {

 public:
  using reference_type = BooleanReference<T>;
  using value_type = BooleanValue<T>;

 public:
  static BooleanValue<T> make() {
    return BooleanValue<T>(false);
  }

  static BooleanReference<T> make(State &container, size_t offset) {
    return BooleanReference<T>(container, offset);
  }

  static const BooleanReference<T> make(const State &container, size_t offset) {
    return BooleanReference<T>(const_cast<State&>(container), offset);
  }

  Boolean &operator=(const Boolean &other) {
    set_value(other.get_value());
    return *this;
  }

  Boolean &operator=(bool other) {
    set_value(other);
    return *this;
  }

  operator bool() const {
    return get_value();
  }

  size_t zero_based_value() const {
    return get_value() ? 1 : 0;
  }

  virtual bool get_value() const = 0;

  virtual void set_value(bool v) = 0;

  void print(FILE *f, const char *title) const {
    fprint(f, "%s = %s", title, get_value() ? "true" : "false");
  }

  static constexpr size_t count() {
    return 2; // "false" and "true"
  }

  static constexpr size_t width() {
    return 1; // we can represent a bool in 1 bit
  }
};
}

namespace {
template<typename>
class BooleanReference : public Boolean<> {

 public:
  State *container;
  const size_t offset;

 public:
  BooleanReference() = delete;
  BooleanReference(State &container_, size_t offset_):
    container(&container_), offset(offset_) { }
  BooleanReference(const BooleanReference&) = default;
  BooleanReference(BooleanReference&&) = default;

  using Boolean<>::operator=;

  BooleanReference &operator=(const BooleanReference &other) {
    return static_cast<BooleanReference&>(Boolean<>::operator=(other));
  }

  bool get_value() const final {
    return container->read(offset, 1);
  }

  void set_value(bool v) final {
    container->write(offset, 1, v);
  }
};
}

namespace {
template<typename>
class BooleanValue : public Boolean<> {

 public:
  bool value;

 public:
  BooleanValue() = delete;
  BooleanValue(bool value_): value(value_) { }
  BooleanValue(const BooleanValue&) = default;
  BooleanValue(BooleanValue&&) = default;

  using Boolean<>::operator=;

  BooleanValue &operator=(const BooleanValue &other) {
    return static_cast<BooleanValue&>(Boolean<>::operator=(other));
  }

  bool get_value() const final {
    return value;
  }

  void set_value(bool v) final {
    value = v;
  }
};
}

namespace {
using ru_u_boolean = Boolean<>;
}
[[gnu::unused]] static const BooleanValue<> ru_u_false(false);
[[gnu::unused]] static const BooleanValue<> ru_u_true(true);

namespace {
template<char... MEMBERS>
class EnumReference;
}

namespace {
template<char... MEMBERS>
class EnumValue;
}

namespace {
template<char... MEMBERS>
class Enum {

 public:
  using reference_type = EnumReference<MEMBERS...>;
  using value_type = EnumValue<MEMBERS...>;

 public:
  static EnumValue<MEMBERS...> make(uint64_t value) {
    return EnumValue<MEMBERS...>(value);
  }

  static EnumReference<MEMBERS...> make(State &container, size_t offset) {
    return EnumReference<MEMBERS...>(container, offset);
  }

  static const EnumReference<MEMBERS...> make(const State &container, size_t offset) {
    return EnumReference<MEMBERS...>(const_cast<State&>(container), offset);
  }

  Enum &operator=(const Enum &other) {
    set_value(other.get_value());
    return *this;
  }

  bool operator==(const Enum &other) const {
    return get_value() == other.get_value();
  }

  bool operator!=(const Enum &other) const {
    return get_value() != other.get_value();
  }

  virtual uint64_t get_value() const = 0;
  virtual void set_value(uint64_t v) = 0;

  void print(FILE *f, const char *title) const {
    fprint(f, "%s = ", title);
    uint64_t v = get_value();
    uint64_t i = 0;
    for (char c : std::array<char, sizeof...(MEMBERS)>{MEMBERS...}) {
      if (c == ',') {
        if (v == i) {
          break;
        }
        i++;
      } else if (v == i) {
        fprint(f, "%c", c);
      }
    }
    ASSERT(v == i && "illegal out-of-range value stored in enum");
  }

  static constexpr size_t count() {
    return sizeof...(MEMBERS) == 0
      ? 0
      : count_commas(std::array<char, sizeof...(MEMBERS)>{MEMBERS...}) + 1;
  }

  static constexpr size_t width() {
    return count() < 2 ? 0 : sizeof(unsigned long long) * CHAR_BIT - __builtin_clzll(count() - 1);
  }

  // XXX: This function is only here so we can define count() as a C++11 constexpr.
  // Perhaps we can find another way around this.
  static constexpr size_t count_commas(const std::array<char, sizeof...(MEMBERS)> &cs) {
    return std::count_if(cs.begin(), cs.end(), [](char c){ return c == ','; });
  }
};
}

namespace {
template<char... MEMBERS>
class EnumReference : public Enum<MEMBERS...> {

 public:
  State *container;
  const size_t offset;

 public:
  EnumReference() = delete;
  EnumReference(State &container_, size_t offset_):
    container(&container_), offset(offset_) { }
  EnumReference(const EnumReference&) = default;
  EnumReference(EnumReference&&) = default;

  using Enum<MEMBERS...>::operator=;

  EnumReference &operator=(const EnumReference &other) {
    return static_cast<EnumReference&>(Enum<MEMBERS...>::operator=(other));
  }

  uint64_t get_value() const final {
    return container->read(offset, width());
  }

  void set_value(uint64_t v) final {
    container->write(offset, width(), v);
  }

  static constexpr size_t count() {
    return Enum<MEMBERS...>::count();
  }

  static constexpr size_t width() {
    return Enum<MEMBERS...>::width();
  }
};
}

namespace {
template<char... MEMBERS>
class EnumValue : public Enum<MEMBERS...> {

 public:
  uint64_t value;

 public:
  EnumValue() = delete;
  EnumValue(uint64_t value_): value(value_) { }
  EnumValue(const EnumValue&) = default;
  EnumValue(EnumValue&&) = default;

  using Enum<MEMBERS...>::operator=;

  EnumValue &operator=(const EnumValue &other) {
    return static_cast<EnumValue&>(Enum<MEMBERS...>::operator=(other));
  }

  uint64_t get_value() const final {
    return value;
  }

  void set_value(uint64_t v) final {
    value = v;
  }

  static constexpr size_t count() {
    return Enum<MEMBERS...>::count();
  }

  static constexpr size_t width() {
    return Enum<MEMBERS...>::width();
  }
};
}

namespace {
template<typename... FIELDS>
class RecordReference;
}

namespace {
template<typename... FIELDS>
class RecordValue;
}

namespace {
template<typename... FIELDS>
class Record {

 public:
  using reference_type = RecordReference<FIELDS...>;
  using value_type = RecordValue<FIELDS...>;

 public:
  static reference_type make(State &s, size_t offset) {
    return reference_type(s, offset);
  }

  static const reference_type make(const State &s, size_t offset) {
    return reference_type(const_cast<State&>(s), offset);
  }

  virtual std::tuple<FIELDS*...> get() = 0;
  virtual std::tuple<const FIELDS*...> get() const = 0;

  void print(FILE *, const char*) const {
    ASSERT(!"TODO");
  }
};
}

namespace {
template<typename... FIELDS>
class RecordReference : public Record<FIELDS...> {

 public:
  State *container;
  const size_t offset;
  std::tuple<typename FIELDS::reference_type...> fields;

 public:
  RecordReference() = delete;
  RecordReference(State &container_, size_t offset_):
    container(&container_), offset(offset_), fields{FIELDS::make(container_, offset_)...} {
    // FIXME: It's not correct to just pass 'offset_' to FIELDS::make
  }

  std::tuple<FIELDS*...> get() final {
    ASSERT(!"TODO");
  }

  std::tuple<const FIELDS*...> get() const final {
    ASSERT(!"TODO");
  }
};
}

namespace {
template<typename... FIELDS>
class RecordValue : public Record<FIELDS...> {

 public:
  std::tuple<typename FIELDS::value_type...> fields;

  std::tuple<FIELDS*...> get() final {
    return make_tuple(std::make_index_sequence<sizeof...(FIELDS)>{});
  }

  std::tuple<const FIELDS*...> get() const final {
    return make_tuple(std::make_index_sequence<sizeof...(FIELDS)>{});
  }

 private:
  template<size_t... IS>
  std::tuple<FIELDS*...> make_tuple(std::index_sequence<IS...>) {
    return std::make_tuple(&fields[IS]...);
  }

  template<size_t... IS>
  std::tuple<const FIELDS*...> make_tuple(std::index_sequence<IS...>) const {
    return std::make_tuple(&fields[IS]...);
  }

};
}
