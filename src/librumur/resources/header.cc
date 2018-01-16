#include <bitset>
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

 private:
  StateBase(const StateBase *s): data(s->data), previous(s) { }
};

template<typename STATE>
struct StartStateBase {

 public:
  const std::string name;
  const std::function<void(STATE&)> body;

 public:
  StartStateBase(const std::string &name_, std::function<void(STATE&)> body_):
    name(name_), body(body_) { }
};

template<typename STATE>
struct InvariantBase {

 public:
  const std::string name;
  const std::function<bool(const STATE&)> guard;

 public:
  InvariantBase(const std::string &name_, std::function<bool(const STATE&)> guard_):
    name(name_), guard(guard_) { }
};

template<typename STATE>
struct RuleBase {

 public:
  const std::string name;
  const std::function<bool(const STATE&)> guard;
  const std::function<void(STATE&)> body;

 public:
  RuleBase(const std::string &name_, std::function<bool(const STATE&)> guard_,
    std::function<void(STATE&)> body_):
    name(name_), guard(guard_), body(body_) { }
};

/* An exception that is thrown that is not related to a specific current state.
 * This is used within infrastructure code.
 */
class Error : public std::runtime_error {

 public:
  using std::runtime_error::runtime_error;

};

/* An exception that is thrown at runtime if an error is detected during model
 * checking.
 */
template<typename STATE>
class ModelErrorBase : public std::runtime_error {

  public:
    const STATE *state;

    ModelErrorBase(const std::string &message, const STATE *state_ = nullptr):
      std::runtime_error(message), state(state_) {
    }

    ModelErrorBase(const ModelErrorBase &e, const STATE *state_):
      std::runtime_error(e.what()), state(state_) {
    }

    ModelErrorBase(const ModelErrorBase&) = default;
    ModelErrorBase(ModelErrorBase&&) = default;

};

[[gnu::unused]] static int64_t add(int64_t a, int64_t b);
[[gnu::unused]] static int64_t sub(int64_t a, int64_t b);
[[gnu::unused]] static int64_t mul(int64_t a, int64_t b);
[[gnu::unused]] static int64_t divide(int64_t a, int64_t b);
[[gnu::unused]] static int64_t mod(int64_t a, int64_t b);
[[gnu::unused]] static int64_t negate(int64_t a);

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
};

template<int64_t MIN, int64_t MAX>
struct RangeBase {

 public:
  int64_t value;

 public:
  RangeBase() = delete;
  RangeBase(int64_t value_): value(value_) { }
  RangeBase(const RangeBase&) = default;
  RangeBase(RangeBase&&) = default;
  RangeBase &operator=(const RangeBase&) = default;
  RangeBase &operator=(RangeBase&&) = default;

  RangeBase operator+(const RangeBase &other) const {
    return add(value, other.value);
  }

  RangeBase operator+(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of the range");
    }
    int64_t v = add(value, other.value);
    if (v < MIN || v > MAX) {
      throw Error("result of addition is out of range");
    }
    return v;
  }

  RangeBase operator-(const RangeBase &other) const {
    return sub(value, other.value);
  }

  RangeBase operator-(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of the range");
    }
    int64_t v = sub(value, other.value);
    if (v < MIN || v > MAX) {
      throw Error("result of subtraction is out of range");
    }
    return v;
  }

  RangeBase operator*(const RangeBase &other) const {
    return mul(value, other.value);
  }

  RangeBase operator*(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in multiplication");
    }
    int64_t v = mul(value, other.value);
    if (v < MIN || v > MAX) {
      throw Error("result of multiplication is out of range");
    }
    return v;
  }

  RangeBase operator/(const RangeBase &other) const {
    return divide(value, other.value);
  }

  RangeBase operator/(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in division");
    }
    int64_t v = divide(value, other.value);
    if (v < MIN || v > MAX) {
      throw Error("result of division is out of range");
    }
    return v;
  }

  RangeBase operator%(const RangeBase &other) const {
    return mod(value, other.value);
  }

  RangeBase operator%(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in mod");
    }
    int64_t v = mod(value, other.value);
    if (v < MIN || v > MAX) {
      throw Error("result of mod is out of range");
    }
    return v;
  }

  bool operator<(const RangeBase &other) const {
    return value < other.value;
  }

  bool operator<(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in <");
    }
    return value < other.value;
  }

  bool operator>(const RangeBase &other) const {
    return value > other.value;
  }

  bool operator>(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in >");
    }
    return value > other.value;
  }

  bool operator==(const RangeBase &other) const {
    return value == other.value;
  }

  bool operator==(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in ==");
    }
    return value == other.value;
  }

  bool operator!=(const RangeBase &other) const {
    return value != other.value;
  }

  bool operator!=(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in !=");
    }
    return value != other.value;
  }

  bool operator<=(const RangeBase &other) const {
    return value <= other.value;
  }

  bool operator<=(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in <=");
    }
    return value <= other.value;
  }

  bool operator>=(const RangeBase &other) const {
    return value >= other.value;
  }

  bool operator>=(const Number &other) const {
    if (other.value < MIN || other.value > MAX) {
      throw Error(std::to_string(other.value) + " is out of range in >=");
    }
    return value >= other.value;
  }
};

template<int64_t MIN, int64_t MAX>
static RangeBase<MIN, MAX> operator+(const Number &a, const RangeBase<MIN, MAX> &b) {
  return b + a;
}

template<int64_t MIN, int64_t MAX>
static RangeBase<MIN, MAX> operator-(const Number &a, const RangeBase<MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range for subtraction");
  }
  int64_t v = sub(a.value, b.value);
  if (v < MIN || v > MAX) {
    throw Error("result of subtraction is out of range");
  }
  return v;
}

template<int64_t MIN, int64_t MAX>
static RangeBase<MIN, MAX> operator*(const Number &a, const RangeBase<MIN, MAX> &b) {
  return b * a;
}

template<int64_t MIN, int64_t MAX>
static RangeBase<MIN, MAX> operator/(const Number &a, const RangeBase<MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in division");
  }
  int64_t v = divide(a.value, b.value);
  if (v < MIN || v > MAX) {
    throw Error("result of division is out of range");
  }
  return v;
}

template<int64_t MIN, int64_t MAX>
static RangeBase<MIN, MAX> operator%(const Number &a, const RangeBase<MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in mod");
  }
  int64_t v = mod(a.value, b.value);
  if (v < MIN || v > MAX) {
    throw Error("result of mod is out of range");
  }
  return v;
}

template<int64_t MIN, int64_t MAX>
static bool operator<(const Number &a, const RangeBase<MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in <");
  }
  return a.value < b.value;
}

template<int64_t MIN, int64_t MAX>
static bool operator>(const Number &a, const RangeBase<MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in >");
  }
  return a.value > b.value;
}

template<int64_t MIN, int64_t MAX>
static bool operator==(const Number &a, const RangeBase<MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in ==");
  }
  return a.value == b.value;
}

template<int64_t MIN, int64_t MAX>
static bool operator!=(const Number &a, const RangeBase<MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in !=");
  }
  return a.value != b.value;
}

template<int64_t MIN, int64_t MAX>
static bool operator<=(const Number &a, const RangeBase<MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in <=");
  }
  return a.value <= b.value;
}

template<int64_t MIN, int64_t MAX>
static bool operator>=(const Number &a, const RangeBase<MIN, MAX> &b) {
  if (a.value < MIN || a.value > MAX) {
    throw Error(std::to_string(a.value) + " is out of range in >=");
  }
  return a.value >= b.value;
}
