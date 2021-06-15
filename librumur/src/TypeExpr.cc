#include "../../common/isa.h"
#include <cassert>
#include <cstddef>
#include <gmpxx.h>
#include <iostream>
#include <limits.h>
#include <memory>
#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/Number.h>
#include <rumur/Ptr.h>
#include <rumur/TypeExpr.h>
#include <rumur/except.h>
#include <rumur/traverse.h>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace rumur {

TypeExpr::TypeExpr(const location &loc_) : Node(loc_) {}

bool TypeExpr::is_simple() const { return false; }

Ptr<TypeExpr> TypeExpr::resolve() const { return Ptr<TypeExpr>(clone()); }

std::string TypeExpr::lower_bound() const {
  throw Error("complex types do not have valid lower bounds", loc);
}

std::string TypeExpr::upper_bound() const {
  throw Error("complex types do not have valid upper bounds", loc);
}

mpz_class TypeExpr::width() const {
  mpz_class c = count();

  // If there are 0 or 1 values of this type, its width is trivial.
  if (c <= 1)
    return 0;

  /* Otherwise, we need the number of bits required to represent the largest
   * value.
   */
  mpz_class largest(c - 1);
  mpz_class bits(0);
  while (largest != 0) {
    bits++;
    largest >>= 1;
  }
  return bits;
}

bool TypeExpr::constant() const {
  assert(!is_simple() && "TypeExpr::constant invoked for simple type; missing "
                         "TypeExpr::constant override?");

  throw Error("complex types do not have bounds to query", loc);
}

// Compare two types for equality. This is implemented here rather than
// operator== because its semantics are not exactly what you would expect from
// operator== and we do not want to expose it to other files.
static bool equal(const TypeExpr &t1, const TypeExpr &t2) {

  class Equater : public ConstTypeTraversal {

  private:
    const Ptr<TypeExpr> t;

  public:
    bool result = true;

    explicit Equater(const TypeExpr &type) : t(type.resolve()) {}

    void visit_array(const Array &n) final {
      if (auto a = dynamic_cast<const Array *>(t.get())) {
        result &= equal(*a->index_type, *n.index_type);
        result &= equal(*a->element_type, *n.element_type);
      } else {
        result = false;
      }
    }

    void visit_enum(const Enum &n) final {
      if (auto e = dynamic_cast<const Enum *>(t.get())) {
        for (auto it = e->members.begin(), it2 = n.members.begin();;
             it++, it2++) {
          if (it == e->members.end()) {
            result &= it2 == n.members.end();
            break;
          }
          if (it2 == n.members.end()) {
            result = false;
            break;
          }
          result &= it->first == it2->first;
        }
      } else {
        result = false;
      }
    }

    void visit_range(const Range &n) final {
      if (auto r = dynamic_cast<const Range *>(t.get())) {
        result = r->min->constant_fold() == n.min->constant_fold() &&
                 r->max->constant_fold() == n.max->constant_fold();
      } else {
        result = false;
      }
    }

    void visit_record(const Record &n) final {
      if (auto r = dynamic_cast<const Record *>(t.get())) {
        for (auto it = r->fields.begin(), it2 = n.fields.begin();;
             it++, it2++) {
          if (it == r->fields.end()) {
            result &= it2 == n.fields.end();
            break;
          }
          if (it2 == n.fields.end()) {
            result = false;
            break;
          }
          result &= (*it)->name == (*it2)->name &&
                    equal(*(*it)->get_type(), *(*it2)->get_type());
        }
      } else {
        result = false;
      }
    }

    void visit_scalarset(const Scalarset &n) final {
      if (auto s = dynamic_cast<const Scalarset *>(t.get())) {
        result = s->bound->constant_fold() == n.bound->constant_fold();
      } else {
        result = false;
      }
    }

    void visit_typeexprid(const TypeExprID &n) final { dispatch(*n.referent); }
  };

  Equater eq(t1);
  eq.dispatch(t2);
  return eq.result;
}

bool TypeExpr::coerces_to(const TypeExpr &other) const {

  const Ptr<TypeExpr> t1 = resolve();
  const Ptr<TypeExpr> t2 = other.resolve();

  if (isa<Range>(t1) && isa<Range>(t2))
    return true;

  return equal(*t1, *t2);
}

bool TypeExpr::is_boolean() const { return false; }

Range::Range(const Ptr<Expr> &min_, const Ptr<Expr> &max_, const location &loc_)
    : TypeExpr(loc_), min(min_), max(max_) {

  if (min == nullptr) {
    // FIXME: avoid hard coding INT64 limits here
    // FIXME: this isn't even the right limit because of - overflowing grr...
    min = Ptr<Number>::make(mpz_class("-9223372036854775807"), location());
  }

  if (max == nullptr) {
    max = Ptr<Number>::make(mpz_class("9223372036854775807"), location());
  }
}

Range *Range::clone() const { return new Range(*this); }

void Range::visit(BaseTraversal &visitor) { visitor.visit_range(*this); }

void Range::visit(ConstBaseTraversal &visitor) const {
  visitor.visit_range(*this);
}

mpz_class Range::count() const {
  mpz_class lb = min->constant_fold();
  mpz_class ub = max->constant_fold();
  return ub - lb + 2;
}

bool Range::is_simple() const { return true; }

void Range::validate() const {
  if (!min->constant())
    throw Error("lower bound of range is not a constant", min->loc);

  if (!max->constant())
    throw Error("upper bound of range is not a constant", max->loc);

  if (max->constant_fold() < min->constant_fold())
    throw Error("upper bound of range is less than lower bound", loc);
}

std::string Range::lower_bound() const {
  return "VALUE_C(" + min->constant_fold().get_str() + ")";
}

std::string Range::upper_bound() const {
  return "VALUE_C(" + max->constant_fold().get_str() + ")";
}

std::string Range::to_string() const {
  return min->to_string() + ".." + max->to_string();
}

bool Range::constant() const { return min->constant() && max->constant(); }

Scalarset::Scalarset(const Ptr<Expr> &bound_, const location &loc_)
    : TypeExpr(loc_), bound(bound_) {}

Scalarset *Scalarset::clone() const { return new Scalarset(*this); }

void Scalarset::visit(BaseTraversal &visitor) {
  visitor.visit_scalarset(*this);
}

void Scalarset::visit(ConstBaseTraversal &visitor) const {
  visitor.visit_scalarset(*this);
}

mpz_class Scalarset::count() const {
  mpz_class b = bound->constant_fold();
  assert(b > 0 && "non-positive bound for scalarset");

  return b + 1;
}

bool Scalarset::is_simple() const { return true; }

void Scalarset::validate() const {
  if (!bound->constant())
    throw Error("bound of scalarset is not a constant", bound->loc);

  if (bound->constant_fold() <= 0)
    throw Error("bound of scalarset is not positive", bound->loc);
}

std::string Scalarset::lower_bound() const { return "VALUE_C(0)"; }

std::string Scalarset::upper_bound() const {
  mpz_class b = bound->constant_fold() - 1;
  return "VALUE_C(" + b.get_str() + ")";
}

std::string Scalarset::to_string() const {
  return "scalarset(" + bound->to_string() + ")";
}

bool Scalarset::constant() const { return bound->constant(); }

Enum::Enum(const std::vector<std::pair<std::string, location>> &members_,
           const location &loc_)
    : TypeExpr(loc_), members(members_) {}

Enum *Enum::clone() const { return new Enum(*this); }

void Enum::visit(BaseTraversal &visitor) { visitor.visit_enum(*this); }

void Enum::visit(ConstBaseTraversal &visitor) const {
  visitor.visit_enum(*this);
}

mpz_class Enum::count() const {
  mpz_class members_size = members.size();
  return members_size + 1;
}

bool Enum::is_simple() const { return true; }

void Enum::validate() const {
  std::unordered_set<std::string> ms;
  for (const std::pair<std::string, location> &member : members) {
    auto it = ms.insert(member.first);
    if (!it.second)
      throw Error("duplicate enum member \"" + member.first + "\"",
                  member.second);
  }
}

std::string Enum::lower_bound() const { return "VALUE_C(0)"; }

std::string Enum::upper_bound() const {
  mpz_class size = members.size();
  if (size > 0)
    size--;
  return "VALUE_C(" + size.get_str() + ")";
}

std::string Enum::to_string() const {
  std::string s = "enum { ";
  bool first = true;
  for (const std::pair<std::string, location> &m : members) {
    if (!first)
      s += ", ";
    s += m.first;
    first = false;
  }
  return s + " }";
}

bool Enum::constant() const {
  // enums always have a known constant bound
  return true;
}

bool Enum::is_boolean() const {
  // the boolean literals cannot be shadowed, so we simply need to check if our
  // members are “false” and “true”
  return members.size() == 2 && members[0].first == "false" &&
         members[1].first == "true";
}

Record::Record(const std::vector<Ptr<VarDecl>> &fields_, const location &loc_)
    : TypeExpr(loc_), fields(fields_) {

  std::unordered_set<std::string> names;
  for (const Ptr<VarDecl> &f : fields) {
    if (!names.insert(f->name).second)
      throw Error("duplicate field name \"" + f->name + "\"", f->loc);
  }
}

Record *Record::clone() const { return new Record(*this); }

void Record::visit(BaseTraversal &visitor) { visitor.visit_record(*this); }

void Record::visit(ConstBaseTraversal &visitor) const {
  visitor.visit_record(*this);
}

mpz_class Record::width() const {
  mpz_class s = 0;
  for (const Ptr<VarDecl> &v : fields)
    s += v->type->width();
  return s;
}

mpz_class Record::count() const {
  mpz_class s = 1;
  for (const Ptr<VarDecl> &v : fields)
    s *= v->type->count();
  return s;
}

std::string Record::to_string() const {
  std::string s = "record ";
  for (const Ptr<VarDecl> &v : fields)
    s += v->name + " : " + v->type->to_string() + "; ";
  return s + "endrecord";
}

Array::Array(const Ptr<TypeExpr> &index_type_,
             const Ptr<TypeExpr> &element_type_, const location &loc_)
    : TypeExpr(loc_), index_type(index_type_), element_type(element_type_) {}

Array *Array::clone() const { return new Array(*this); }

void Array::visit(BaseTraversal &visitor) { visitor.visit_array(*this); }

void Array::visit(ConstBaseTraversal &visitor) const {
  visitor.visit_array(*this);
}

mpz_class Array::width() const {

  mpz_class i = index_type->count();
  mpz_class e = element_type->width();

  assert(i >= 1 && "index count apparently does not include undefined");
  i--;

  return i * e;
}

mpz_class Array::count() const {

  mpz_class i = index_type->count();
  mpz_class e = element_type->count();

  assert(i >= 1 && "index count apparently does not include undefined");
  i--;

  if (i == 0)
    return 0;

  mpz_class s = 1;
  for (size_t j = 0; j < i; j++)
    s *= e;
  return s;
}

void Array::validate() const {
  if (!index_type->is_simple())
    throw Error("array indices must be simple types", loc);
}

std::string Array::to_string() const {
  return "array [" + index_type->to_string() + "] of " +
         element_type->to_string();
}

TypeExprID::TypeExprID(const std::string &name_, const Ptr<TypeDecl> &referent_,
                       const location &loc_)
    : TypeExpr(loc_), name(name_), referent(referent_) {}

TypeExprID *TypeExprID::clone() const { return new TypeExprID(*this); }

void TypeExprID::visit(BaseTraversal &visitor) {
  visitor.visit_typeexprid(*this);
}

void TypeExprID::visit(ConstBaseTraversal &visitor) const {
  visitor.visit_typeexprid(*this);
}

mpz_class TypeExprID::width() const {
  if (referent == nullptr)
    throw Error("unresolved type symbol \"" + name + "\"", loc);
  return referent->value->width();
}

mpz_class TypeExprID::count() const {
  if (referent == nullptr)
    throw Error("unresolved type symbol \"" + name + "\"", loc);
  return referent->value->count();
}

bool TypeExprID::is_simple() const {
  if (referent == nullptr)
    throw Error("unresolved type symbol \"" + name + "\"", loc);
  return referent->value->is_simple();
}

Ptr<TypeExpr> TypeExprID::resolve() const {
  if (referent == nullptr)
    throw Error("unresolved type symbol \"" + name + "\"", loc);
  return referent->value->resolve();
}

void TypeExprID::validate() const {
  if (referent == nullptr)
    throw Error("unresolved type symbol \"" + name + "\"", loc);
}

std::string TypeExprID::lower_bound() const {
  if (referent == nullptr)
    throw Error("unresolved type symbol \"" + name + "\"", loc);
  return referent->value->lower_bound();
}

std::string TypeExprID::upper_bound() const {
  if (referent == nullptr)
    throw Error("unresolved type symbol \"" + name + "\"", loc);
  return referent->value->upper_bound();
}

std::string TypeExprID::to_string() const { return name; }

bool TypeExprID::constant() const {
  if (referent == nullptr)
    throw Error("unresolved type symbol \"" + name + "\"", loc);
  return referent->value->constant();
}

} // namespace rumur
