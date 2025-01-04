#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>

#ifndef RUMUR_API
#define RUMUR_API __attribute__((visibility("default")))
#endif

namespace rumur {

// An implementation of a managed pointer that understands *::clone()
template <typename TARGET> class RUMUR_API Ptr {

private:
  std::unique_ptr<TARGET> t = std::unique_ptr<TARGET>(nullptr);

public:
  Ptr() = default;

  Ptr(std::nullptr_t) {}

  explicit Ptr(TARGET *t_) : t(t_) {}

  Ptr(const Ptr &p) : t(p.t == nullptr ? nullptr : p.t->clone()) {}

  Ptr(Ptr &&p) noexcept {
    using std::swap;
    swap(t, p.t);
  }

  template <typename SUBTYPE>
  Ptr(const Ptr<SUBTYPE> &p) : t(p.get() == nullptr ? nullptr : p->clone()) {}

  Ptr &operator=(const Ptr &p) {
    t.reset(p.t == nullptr ? nullptr : p.t->clone());
    return *this;
  }

  Ptr &operator=(Ptr &&p) noexcept {
    using std::swap;
    swap(t, p.t);
    return *this;
  }

  template <typename SUBTYPE> Ptr &operator=(const Ptr<SUBTYPE> &p) {
    t.reset(p.get() == nullptr ? nullptr : p->clone());
    return *this;
  }

  const TARGET *get() const { return t.get(); }

  TARGET *get() { return t.get(); }

  template <typename SUBTYPE> Ptr<SUBTYPE> narrow() {

    // can we downcast to the requested type?
    auto subtype = dynamic_cast<SUBTYPE *>(t.get());
    if (subtype == nullptr)
      return Ptr<SUBTYPE>(nullptr);

    // if so, steal our pointer
    (void)t.release();
    return Ptr<SUBTYPE>(subtype);
  }

  TARGET &operator*() {
    assert(t != nullptr && "dereferencing a null pointer");
    return *t;
  }

  const TARGET &operator*() const {
    assert(t != nullptr && "dereferencing a null pointer");
    return *t;
  }

  TARGET *operator->() {
    assert(t != nullptr && "dereferencing a null pointer");
    return t.get();
  }

  const TARGET *operator->() const {
    assert(t != nullptr && "dereferencing a null pointer");
    return t.get();
  }

  bool operator==(const Ptr &other) const { return t == other.t; }

  bool operator==(const TARGET *other) const { return t.get() == other; }

  bool operator!=(const Ptr &other) const { return t != other.t; }

  bool operator!=(const TARGET *other) const { return t.get() != other; }

  template <typename... Args> static Ptr<TARGET> make(Args &&...args) {
    return Ptr<TARGET>(new TARGET(std::forward<Args>(args)...));
  }
};

} // namespace rumur
