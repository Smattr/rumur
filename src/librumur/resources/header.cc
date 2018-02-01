#ifndef __OPTIMIZE__
  #warning you are compiling without optimizations enabled. I would suggest -O3 -fwhole-program.
#endif

#include <cassert>
#include <ctime>
#include <bitset>
#include <cinttypes>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <limits>
#include <queue>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <unordered_set>
#include <utility>

[[gnu::format(printf, 1, 2)]] static void print(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  (void)vprintf(fmt, ap);
  va_end(ap);
}

[[gnu::format(printf, 2, 3)]] static void fprint(FILE *f, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  (void)vfprintf(f, fmt, ap);
  va_end(ap);
}

template<size_t SIZE_BITS>
struct StateBase {
  std::bitset<SIZE_BITS> data;
  const StateBase *previous = nullptr;

 public:
  StateBase() = default;
  StateBase(const StateBase&) = default;
  StateBase(StateBase&&) = default;
  StateBase &operator=(const StateBase&) = default;
  StateBase &operator=(StateBase&&) = default;

  StateBase *duplicate() const {
    return new StateBase(this);
  }

  bool operator==(const StateBase &other) const {
    return data == other.data;
  }

  bool operator!=(const StateBase &other) const {
    return !(*this == other);
  }

  int64_t read(size_t offset, size_t width) const {
    static_assert(sizeof(unsigned long long) >= sizeof(int64_t),
      "cannot read a int64_t out of a std::bitset");
    assert(width <= sizeof(int64_t) * CHAR_BIT && "read of too large value");
    assert(offset <= SIZE_BITS - 1 && "out of bounds read");
    std::bitset<SIZE_BITS> v = (data >> offset) & std::bitset<SIZE_BITS>((UINT64_C(1) << width) - 1);
    if (sizeof(unsigned long) >= sizeof(int64_t)) {
      return v.to_ulong();
    }
    return v.to_ullong();
  }

  void write(size_t offset, size_t width, int64_t value) {
    static_assert(sizeof(unsigned long) >= sizeof(int64_t),
      "cannot write a int64_t to a std::bitset");
    assert(width <= sizeof(int64_t) * CHAR_BIT && "write of too large a value");
    assert(1ul << width > (unsigned long)value && "write of too large a value");
    std::bitset<SIZE_BITS> v = std::bitset<SIZE_BITS>((unsigned long)value) << offset;
    std::bitset<SIZE_BITS> mask = ~(std::bitset<SIZE_BITS>((UINT64_C(1) << width) - 1) << offset);
    data = (data & mask) | v;
  }

  size_t hash() const {
    return std::hash<std::bitset<SIZE_BITS>>{}(data);
  }

 private:
  StateBase(const StateBase *s): data(s->data), previous(s) { }
};

template<typename STATE_T>
struct StartStateBase {

 public:
  const std::string name;
  const std::function<void(STATE_T&)> body;

 public:
  StartStateBase(const std::string &name_, std::function<void(STATE_T&)> body_):
    name(name_), body(body_) { }
};

template<typename STATE_T>
struct InvariantBase {

 public:
  const std::string name;
  const std::function<bool(const STATE_T&)> guard;

 public:
  InvariantBase(const std::string &name_, std::function<bool(const STATE_T&)> guard_):
    name(name_), guard(guard_) { }
};

template<typename STATE_T>
struct RuleBase {

 public:
  const std::string name;
  const std::function<bool(const STATE_T&)> guard;
  const std::function<void(STATE_T&)> body;

 public:
  RuleBase(const std::string &name_, std::function<bool(const STATE_T&)> guard_,
    std::function<void(STATE_T&)> body_):
    name(name_), guard(guard_), body(body_) { }

 private:
  class iterator {
   private:
    const RuleBase &rule;
    STATE_T &origin;
    bool end;

   public:
    iterator(const RuleBase &rule_, STATE_T &origin_, bool end_ = false):
      rule(rule_), origin(origin_), end(end_) {
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

    STATE_T *operator*() const {
      assert(!end);
      STATE_T *d = origin.duplicate();
      rule.body(*d);
      return d;
    }
  };

  class iterable {
   private:
    const RuleBase &rule;
    STATE_T &origin;

   public:
    iterable(const RuleBase &rule_, STATE_T &origin_):
      rule(rule_), origin(origin_) { }

    iterator begin() const {
      return iterator(rule, origin);
    }

    iterator end() const {
      return iterator(rule, origin, true);
    }
  };

 public:
  iterable get_iterable(STATE_T &origin) const {
    return iterable(*this, origin);
  }
};

/* An exception that is thrown that is not related to a specific current state.
 * This is used within infrastructure code.
 */
class Error : public std::runtime_error {

 public:
  using std::runtime_error::runtime_error;

};

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

struct Number {

 public:
  int64_t value;

 public:
  Number() = delete;
  constexpr Number(int64_t value_): value(value_) { }
  Number(const Number&) = default;
  Number(Number&&) = default;
  Number &operator=(const Number&) = default;
  Number &operator=(Number&&) = default;

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
    fprintf(f, "%s = %" PRId64, title, value);
  }
};

template<typename STATE_T, int64_t MIN, int64_t MAX>
struct RangeBase {

 public:
  static const size_t COUNT = MAX - MIN + 1;
  static const size_t SIZE = MAX - MIN == 0 ? 0 : sizeof(unsigned long long) * CHAR_BIT - __builtin_clzll(MAX - MIN);

  const bool in_state = false;
  int64_t value = 0;
  STATE_T *s = nullptr;
  const size_t offset = 0;

 private:
  RangeBase(STATE_T &s_, size_t offset_): in_state(true), s(&s_), offset(offset_) { }

 public:
  RangeBase() = delete;
  RangeBase(int64_t value_): value(value_) { }
  RangeBase(const RangeBase&) = default;
  RangeBase(RangeBase&&) = default;

  RangeBase &operator=(const RangeBase &other) {
    set_value(other.get_value());
    return *this;
  }

  RangeBase &operator=(const Number &other) {
    set_value(other.value);
    return *this;
  }

  static RangeBase make() {
    return RangeBase(MIN);
  }

  static RangeBase make(STATE_T &s, size_t offset) {
    return RangeBase(s, offset);
  }

  static const RangeBase make(const STATE_T &s, size_t offset) {
    return RangeBase(const_cast<STATE_T&>(s), offset);
  }

  size_t zero_based_value() const {
    size_t r;
    if (__builtin_sub_overflow(get_value(), MIN, &r))
      throw Error("overflow in calculating index value");
    return r;
  }

  RangeBase operator+(const RangeBase &other) const {
    return add(get_value(), other.get_value());
  }

  RangeBase operator+(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of the range");
    }
    int64_t v = add(get_value(), other.value);
    if (v < MIN || v > MAX) {
      throw Error("result of addition is out of range");
    }
    return v;
  }

  RangeBase operator-(const RangeBase &other) const {
    return sub(get_value(), other.get_value());
  }

  RangeBase operator-(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of the range");
    }
    int64_t v = sub(get_value(), other.value);
    if (v < MIN || v > MAX) {
      throw Error("result of subtraction is out of range");
    }
    return v;
  }

  RangeBase operator*(const RangeBase &other) const {
    return mul(get_value(), other.get_value());
  }

  RangeBase operator*(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in multiplication");
    }
    int64_t v = mul(get_value(), other.value);
    if (v < MIN || v > MAX) {
      throw Error("result of multiplication is out of range");
    }
    return v;
  }

  RangeBase operator/(const RangeBase &other) const {
    return divide(get_value(), other.get_value());
  }

  RangeBase operator/(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in division");
    }
    int64_t v = divide(get_value(), other.value);
    if (v < MIN || v > MAX) {
      throw Error("result of division is out of range");
    }
    return v;
  }

  RangeBase operator%(const RangeBase &other) const {
    return mod(get_value(), other.get_value());
  }

  RangeBase operator%(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in mod");
    }
    int64_t v = mod(get_value(), other.value);
    if (v < MIN || v > MAX) {
      throw Error("result of mod is out of range");
    }
    return v;
  }

  bool operator<(const RangeBase &other) const {
    return get_value() < other.get_value();
  }

  bool operator<(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in <");
    }
    return get_value() < other.value;
  }

  bool operator>(const RangeBase &other) const {
    return get_value() > other.get_value();
  }

  bool operator>(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in >");
    }
    return get_value() > other.value;
  }

  bool operator==(const RangeBase &other) const {
    return get_value() == other.get_value();
  }

  bool operator==(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in ==");
    }
    return get_value() == other.value;
  }

  bool operator!=(const RangeBase &other) const {
    return get_value() != other.get_value();
  }

  bool operator!=(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in !=");
    }
    return get_value() != other.value;
  }

  bool operator<=(const RangeBase &other) const {
    return get_value() <= other.get_value();
  }

  bool operator<=(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in <=");
    }
    return get_value() <= other.value;
  }

  bool operator>=(const RangeBase &other) const {
    return get_value() >= other.get_value();
  }

  bool operator>=(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in >=");
    }
    return get_value() >= other.value;
  }

  int64_t get_value() const {
    if (in_state) {
      assert(s != nullptr);
      return s->read(offset, SIZE) + MIN;
    }
    return value;
  }

  void set_value(int64_t v) {
    if (in_state) {
      assert(s != nullptr);
      s->write(offset, SIZE, v - MIN);
    } else {
      value = v;
    }
  }

  void print(FILE *f, const char *title) const {
    fprintf(f, "%s = %" PRId64, title, get_value());
  }

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
};

template<typename STATE_T, int64_t MIN, int64_t MAX>
static RangeBase<STATE_T, MIN, MAX> operator+(const Number &a, const RangeBase<STATE_T, MIN, MAX> &b) {
  return b + a;
}

template<typename STATE_T, int64_t MIN, int64_t MAX>
static RangeBase<STATE_T, MIN, MAX> operator-(const Number &a, const RangeBase<STATE_T, MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range for subtraction");
  }
  int64_t v = sub(a.value, b.get_value());
  if (v < MIN || v > MAX) {
    throw Error("result of subtraction is out of range");
  }
  return v;
}

template<typename STATE_T, int64_t MIN, int64_t MAX>
static RangeBase<STATE_T, MIN, MAX> operator*(const Number &a, const RangeBase<STATE_T, MIN, MAX> &b) {
  return b * a;
}

template<typename STATE_T, int64_t MIN, int64_t MAX>
static RangeBase<STATE_T, MIN, MAX> operator/(const Number &a, const RangeBase<STATE_T, MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in division");
  }
  int64_t v = divide(a.value, b.get_value());
  if (v < MIN || v > MAX) {
    throw Error("result of division is out of range");
  }
  return v;
}

template<typename STATE_T, int64_t MIN, int64_t MAX>
static RangeBase<STATE_T, MIN, MAX> operator%(const Number &a, const RangeBase<STATE_T, MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in mod");
  }
  int64_t v = mod(a.value, b.get_value());
  if (v < MIN || v > MAX) {
    throw Error("result of mod is out of range");
  }
  return v;
}

template<typename STATE_T, int64_t MIN, int64_t MAX>
static bool operator<(const Number &a, const RangeBase<STATE_T, MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in <");
  }
  return a.value < b.get_value();
}

template<typename STATE_T, int64_t MIN, int64_t MAX>
static bool operator>(const Number &a, const RangeBase<STATE_T, MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in >");
  }
  return a.value > b.get_value();
}

template<typename STATE_T, int64_t MIN, int64_t MAX>
static bool operator==(const Number &a, const RangeBase<STATE_T, MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in ==");
  }
  return a.value == b.get_value();
}

template<typename STATE_T, int64_t MIN, int64_t MAX>
static bool operator!=(const Number &a, const RangeBase<STATE_T, MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in !=");
  }
  return a.value != b.get_value();
}

template<typename STATE_T, int64_t MIN, int64_t MAX>
static bool operator<=(const Number &a, const RangeBase<STATE_T, MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in <=");
  }
  return a.value <= b.get_value();
}

template<typename STATE_T, int64_t MIN, int64_t MAX>
static bool operator>=(const Number &a, const RangeBase<STATE_T, MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in >=");
  }
  return a.value >= b.get_value();
}

template<typename STATE_T, typename INDEX_T, typename ELEMENT_T>
class Array {

 public:
  const bool in_state = false;
  ELEMENT_T data[INDEX_T::COUNT];
  STATE_T *s = nullptr;
  const size_t offset = 0;

 public:
  // TODO: support for state references below
  ELEMENT_T &operator[](const INDEX_T &index) {
    return data[index.zero_based_value()];
  }

  const ELEMENT_T &operator[](const INDEX_T &index) const {
    return data[index.zero_based_value()];
  }

  ELEMENT_T &operator[](const Number &index) {
    return data[INDEX_T(index.value).zero_based_value()];
  }

  const ELEMENT_T &operator[](const Number &index) const {
    return data[INDEX_T(index.value).zero_based_value()];
  }
};

template<typename STATE_T>
class boolean {

 public:
  const bool in_state = false;
  bool value = false;
  STATE_T *s = nullptr;
  const size_t offset = 0;

 private:
  boolean(STATE_T &s_, size_t offset_): in_state(true), s(&s_), offset(offset_) { }

 public:
  boolean() = delete;
  boolean(bool value_): value(value_) { }

  static boolean make() {
    return boolean(false);
  }

  boolean(const boolean&) = default;
  boolean(boolean&&) = default;

  boolean &operator=(const boolean &other) {
    set_value(other.get_value());
    return *this;
  }

  static boolean make(STATE_T &s, size_t offset) {
    return boolean(s, offset);
  }

  static const boolean make(const STATE_T &s, size_t offset) {
    return boolean(const_cast<STATE_T&>(s), offset);
  }

  operator bool() const {
    return get_value();
  }

  bool get_value() const {
    if (in_state) {
      return s->read(offset, 1);
    }
    return value;
  }

  void set_value(bool v) {
    if (in_state) {
      s->write(offset, 1, v);
    } else {
      value = v;
    }
  }

  void print(FILE *f, const char *title) const {
    fprintf(f, "%s = %s", title, get_value());
  }
};
