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
#include <type_traits>

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

class BitBlock {

 public:
  virtual int64_t read(size_t offset, size_t width) const = 0;
  virtual void write(size_t offset, size_t width, int64_t value) = 0;

  virtual ~BitBlock() { }
};

template<size_t SIZE>
static int64_t read_bits(const std::bitset<SIZE> &data, size_t offset, size_t width) {
  static_assert(sizeof(unsigned long long) >= sizeof(int64_t),
    "cannot read a int64_t out of a std::bitset");
  ASSERT(width <= sizeof(int64_t) * CHAR_BIT && "read of too large value");
  ASSERT(offset <= SIZE - 1 && "out of bounds read");
  std::bitset<SIZE> v = (data >> offset) & std::bitset<SIZE>((UINT64_C(1) << width) - 1);
  if (sizeof(unsigned long) >= sizeof(int64_t)) {
    return v.to_ulong();
  }
  return v.to_ullong();
}

template<size_t SIZE>
static void write_bits(std::bitset<SIZE> &data, size_t offset, size_t width, int64_t value) {
  static_assert(sizeof(unsigned long) >= sizeof(int64_t),
    "cannot write a int64_t to a std::bitset");
  ASSERT(width <= sizeof(int64_t) * CHAR_BIT && "write of too large a value");
  ASSERT(1ul << width > (unsigned long)value && "write of too large a value");
  std::bitset<SIZE> v = std::bitset<SIZE>((unsigned long)value) << offset;
  std::bitset<SIZE> mask = ~(std::bitset<SIZE>((UINT64_C(1) << width) - 1) << offset);
  data = (data & mask) | v;
}

template<size_t SIZE_BITS>
struct StateBase : public BitBlock {
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

  int64_t read(size_t offset, size_t width) const final {
    return read_bits(data, offset, width);
  }

  void write(size_t offset, size_t width, int64_t value) final {
    write_bits(data, offset, width, value);
  }

  size_t hash() const {
    return std::hash<std::bitset<SIZE_BITS>>{}(data);
  }

  virtual ~StateBase() { }

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
      ASSERT(!end);
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

/* Compile-time discrimination as to whether a type is Number. */
template<typename>
struct isaNumber : public std::false_type { };

template<>
struct isaNumber<Number> : public std::true_type { };

template<int64_t MIN, int64_t MAX>
struct RangeReference;

template<int64_t MIN, int64_t MAX>
struct RangeValue;

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

  static RangeReference<MIN, MAX> make(BitBlock &container, size_t offset) {
    return RangeReference<MIN, MAX>(container, offset);
  }

  static const RangeReference<MIN, MAX> make(const BitBlock &container, size_t offset) {
    return RangeReference<MIN, MAX>(const_cast<BitBlock&>(container), offset);
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
    fprintf(f, "%s = %" PRId64, title, get_value());
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

template<typename>
struct isaRange : public std::false_type { };

template<int64_t MIN, int64_t MAX>
struct isaRange<Range<MIN, MAX>> : public std::true_type { };

template<int64_t MIN, int64_t MAX>
struct RangeReference : public Range<MIN, MAX> {

 public:
  BitBlock *container;
  const size_t offset;

 public:
  RangeReference() = delete;
  RangeReference(BitBlock &container_, size_t offset_):
    container(&container_), offset(offset_) { }
  RangeReference(const RangeReference&) = default;
  RangeReference(RangeReference&&) = default;

  using Range<MIN, MAX>::operator=;

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

  int64_t get_value() const final {
    return value;
  }

  void set_value(int64_t v) final {
    value = v;
  }
};

template<typename STATE_T, typename INDEX_T, typename ELEMENT_T>
class ArrayReference;

template<typename STATE_T, typename INDEX_T, typename ELEMENT_T>
class ArrayValue;

template<typename STATE_T, typename INDEX_T, typename ELEMENT_T>
class Array {

 public:
  static const ArrayReference<STATE_T, INDEX_T, ELEMENT_T> make(const STATE_T &s, size_t offset) {
    return ArrayReference<STATE_T, INDEX_T, ELEMENT_T>(const_cast<STATE_T &>(s), offset);
  }

  static ArrayReference<STATE_T, INDEX_T, ELEMENT_T> make(STATE_T &s, size_t offset) {
    return ArrayReference<STATE_T, INDEX_T, ELEMENT_T>(s, offset);
  }

  /* operator[] that takes a Number and is only valid if our index type is a
   * range.
   */
  template<typename = typename std::enable_if<isaRange<INDEX_T>::value>::type>
  ELEMENT_T &operator[](const Number &index) {
    if (index.value < INDEX_T::min() || index.value > INDEX_T::max()) {
      throw Error("out of range access to array element " + std::to_string(index.value));
    }
    return (*this)[index.value];
  }

  template<typename = typename std::enable_if<isaRange<INDEX_T>::value>::type>
  const ELEMENT_T &operator[](const Number &index) const {
    if (index.value < INDEX_T::min() || index.value > INDEX_T::max()) {
      throw Error("out of range access to array element " + std::to_string(index.value));
    }
    return (*this)[index.value];
  }

  ELEMENT_T &operator[](const INDEX_T &index) {
    return (*this)[index.zero_based_value()];
  }

  const ELEMENT_T &operator[](const INDEX_T &index) const {
    return (*this)[index.zero_based_value()];
  }

 private:
  virtual ELEMENT_T &operator[](size_t index) = 0;
  virtual const ELEMENT_T &operator[](size_t index) const = 0;

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

template<typename STATE_T, typename INDEX_T, typename ELEMENT_T>
class ArrayReference : public Array<STATE_T, INDEX_T, ELEMENT_T> {

 public:
  STATE_T *s;
  const size_t offset;
  std::vector<typename ELEMENT_T::reference_type> value;

 public:
  ArrayReference() = delete;
  ArrayReference(STATE_T &s_, size_t offset_): s(&s_), offset(offset_) {
    size_t off = offset;
    for (size_t i = 0; i < INDEX_T::count(); i++) {
      value.push_back(ELEMENT_T::make(*s, off));
      off += ELEMENT_T::width();
    }
  }
  ArrayReference(const ArrayReference&) = default;
  ArrayReference(ArrayReference&&) = default;

  /* FIXME: For some reason the enable_if'd operator[] definitions don't get
   * inherited and we need to repeat them to make them accessible. Is this
   * related to them not depending on the class template parameters?
   */
 public:
  template<typename = typename std::enable_if<isaRange<INDEX_T>::value>::type>
  ELEMENT_T &operator[](const Number &index) {
    return static_cast<Array<STATE_T, INDEX_T, ELEMENT_T>&>(*this)[index];
  }

  template<typename = typename std::enable_if<isaRange<INDEX_T>::value>::type>
  const ELEMENT_T &operator[](const Number &index) const {
    return static_cast<Array<STATE_T, INDEX_T, ELEMENT_T>&>(*this)[index];
  }

 private:
  typename ELEMENT_T::reference_type &operator[](size_t index) final {
    return value[index];
  }

  const typename ELEMENT_T::reference_type &operator[](size_t index) const final {
    return value[index];
  }
};

template<typename STATE_T, typename INDEX_T, typename ELEMENT_T>
class ArrayValue : public Array<STATE_T, INDEX_T, ELEMENT_T> {

 public:
  typename ELEMENT_T::value_type value[INDEX_T::count()];

 public:
  ArrayValue() = delete;

 public:
  template<typename = typename std::enable_if<isaRange<INDEX_T>::value>::type>
  ELEMENT_T &operator[](const Number &index) {
    return Array<STATE_T, INDEX_T, ELEMENT_T>::operator[](index);
  }

  template<typename = typename std::enable_if<isaRange<INDEX_T>::value>::type>
  const ELEMENT_T &operator[](const Number &index) const {
    return Array<STATE_T, INDEX_T, ELEMENT_T>::operator[](index);
  }

 private:
  typename ELEMENT_T::value_type &operator[](size_t index) final {
    return value[index];
  }

  const typename ELEMENT_T::value_type &operator[](size_t index) const final {
    return value[index];
  }
};

template<typename STATE_T>
class BooleanReference;

template<typename STATE_T>
class BooleanValue;

template<typename STATE_T>
class Boolean {

 public:
  static BooleanValue<STATE_T> make() {
    return BooleanValue<STATE_T>(false);
  }

  static BooleanReference<STATE_T> make(STATE_T &s, size_t offset) {
    return BooleanReference<STATE_T>(s, offset);
  }

  static const BooleanReference<STATE_T> make(const STATE_T &s, size_t offset) {
    return BooleanReference<STATE_T>(const_cast<STATE_T&>(s), offset);
  }

  Boolean &operator=(const Boolean &other) {
    set_value(other.get_value());
    return *this;
  }

  operator bool() const {
    return get_value();
  }

  virtual bool get_value() const = 0;

  virtual void set_value(bool v) = 0;

  void print(FILE *f, const char *title) const {
    fprintf(f, "%s = %s", title, get_value());
  }

  static size_t count() {
    return 2; // "false" and "true"
  }

  static constexpr size_t width() {
    return 1; // we can represent a bool in 1 bit
  }
};

template<typename STATE_T>
class BooleanReference : public Boolean<STATE_T> {

 public:
  STATE_T *s;
  const size_t offset;

 public:
  BooleanReference() = delete;
  BooleanReference(STATE_T &s_, size_t offset_): s(&s_), offset(offset_) { }
  BooleanReference(const BooleanReference&&) = default;
  BooleanReference(BooleanReference&&) = default;

  using Boolean<STATE_T>::operator=;

  bool get_value() const final {
    return s->read(offset, 1);
  }

  void set_value(bool v) final {
    s->write(offset, 1, v);
  }
};

template<typename STATE_T>
class BooleanValue : public Boolean<STATE_T> {

 public:
  bool value;

 public:
  BooleanValue() = delete;
  BooleanValue(bool value_): value(value_) { }
  BooleanValue(const BooleanValue&) = default;
  BooleanValue(BooleanValue&&) = default;

  using Boolean<STATE_T>::operator=;

  bool get_value() const final {
    return value;
  }

  void set_value(bool v) final {
    value = v;
  }
};
