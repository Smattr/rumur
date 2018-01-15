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
    StateBase(const StateBase *s): data(s->data), previous(s) {
    }

    bool operator==(const StateBase &other) const {
        return data == other.data;
    }

    bool operator!=(const StateBase &other) const {
        return !(*this == other);
    }
};

template<typename STATE>
struct StartStateBase {
    std::string name;
    std::function<void(STATE&)> body;
};

template<typename STATE>
struct InvariantBase {
    std::string name;
    std::function<bool(const STATE&)> guard;
};

template<typename STATE>
struct RuleBase {
    std::string name;
    std::function<bool(const STATE&)> guard;
    std::function<void(STATE&)> body;
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

class Boolean {

 private:
  bool value;

 public:
  Boolean() = delete;
  Boolean(bool value_): value(value_) { }
  Boolean(const Boolean&) = default;
  Boolean(Boolean&&) = default;
  Boolean &operator=(const Boolean&) = default;
  Boolean &operator=(Boolean&&) = default;

  Boolean operator!() const {
    return !value;
  }

  Boolean operator==(const Boolean &other) const {
    return value == other.value;
  }

  Boolean operator!=(const Boolean &other) const {
    return value != other.value;
  }

  Boolean operator&&(const Boolean &other) const {
    return value && other.value;
  }

  Boolean operator||(const Boolean &other) const {
    return value || other.value;
  }
};

class Number {

 private:
  int64_t value;

 public:
  Number() = delete;
  Number(int64_t value_): value(value_) { }
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

  Boolean operator<(const Number &other) const {
    return value < other.value;
  }

  Boolean operator>(const Number &other) const {
    return value > other.value;
  }

  Boolean operator==(const Number &other) const {
    return value == other.value;
  }

  Boolean operator!=(const Number &other) const {
    return value != other.value;
  }

  Boolean operator<=(const Number &other) const {
    return value <= other.value;
  }

  Boolean operator>=(const Number &other) const {
    return value >= other.value;
  }
};
