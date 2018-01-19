#include <iostream>
#include <limits.h>
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>
#include <vector>

namespace rumur {

static size_t bits_for(unsigned long long v) {
  if (v == 0)
    return 0;
  return sizeof(v) * CHAR_BIT - __builtin_clzll(v);
}

bool TypeExpr::is_simple() const {
  return false;
}

SimpleTypeExpr::SimpleTypeExpr(const location &loc_):
  TypeExpr(loc_) {
}

bool SimpleTypeExpr::is_simple() const {
  return true;
}

Range::Range(Expr *min_, Expr *max_, const location &loc_):
  SimpleTypeExpr(loc_), min(min_), max(max_) {
}

Range::Range(const Range &other):
  SimpleTypeExpr(other), min(other.min->clone()), max(other.max->clone()) {
}

Range &Range::operator=(Range other) {
  swap(*this, other);
  return *this;
}

void swap(Range &x, Range &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.min, y.min);
  swap(x.max, y.max);
}

Range *Range::clone() const {
  return new Range(*this);
}

void Range::validate() const {
  if (!min->constant())
    throw RumurError("lower bound of range is not a constant", min->loc);

  if (!max->constant())
    throw RumurError("upper bound of range is not a constant", max->loc);
}

Range::~Range() {
  delete min;
  delete max;
}

void Range::generate(std::ostream &out) const {
  int64_t lb = min->constant_fold();
  int64_t ub = max->constant_fold();
  // TODO: catch overflow, not constant
  out << "RangeBase<" << lb << "," << ub << ">";
}

size_t Range::size() const {
  int64_t lb;
  try {
    lb = min->constant_fold();
  } catch (RumurError e) {
    throw RumurError("lower bound of range is not constant: ", e);
  }
  int64_t ub;
  try {
    ub = max->constant_fold();
  } catch (RumurError e) {
    throw RumurError("upper bound of range is not constant: ", e);
  }
  uint64_t range = ub;
  if (__builtin_sub_overflow(ub, lb, &range))
    throw RumurError("range calculation overflows uint64_t", loc);
  return bits_for(range);
}

Enum::Enum(const std::vector<std::pair<std::string, location>> &&members_, const location &loc_):
  SimpleTypeExpr(loc_), members(members_) {
}

Enum *Enum::clone() const {
  return new Enum(*this);
}

void Enum::generate(std::ostream &out) const {
  out << "EnumBase<";
  bool first = true;
  for (const std::pair<std::string, location> &m : members) {
    if (!first)
      out << ",";
    out << "\"" << m.first << "\"";
    first = false;
  }
  out << ">";
}

size_t Enum::size() const {
  return bits_for(members.size());
}

Record::Record(std::vector<VarDecl*> &&fields_, const location &loc_):
  TypeExpr(loc_), fields(fields_) {
}

Record::Record(const Record &other):
  TypeExpr(other) {
  for (const VarDecl *v : other.fields)
    fields.push_back(v->clone());
}

Record &Record::operator=(Record other) {
  swap(*this, other);
  return *this;
}

void swap(Record &x, Record &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.fields, y.fields);
}

Record *Record::clone() const {
  return new Record(*this);
}

Record::~Record() {
  for (VarDecl *v : fields)
    delete v;
}

void Record::generate(std::ostream &out) const {
  out << "class:public RecordBase{";
  // TODO
  out << "}";
}

size_t Record::size() const {
  size_t s = 0;
  for (const VarDecl *v : fields) {
    size_t v_size = v->type->size();
    if (__builtin_add_overflow(s, v_size, &s))
      throw RumurError("overflow in calculating size of record", loc);
  }
  return s;
}

Array::Array(TypeExpr *index_type_, TypeExpr *element_type_, const location &loc_):
  TypeExpr(loc_), index_type(index_type_), element_type(element_type_) {
}

Array::Array(const Array &other):
  TypeExpr(other), index_type(other.index_type->clone()),
  element_type(other.element_type->clone()) {
}

Array &Array::operator=(Array other) {
  swap(*this, other);
  return *this;
}

void swap(Array &x, Array &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.index_type, y.index_type);
  swap(x.element_type, y.element_type);
}

Array *Array::clone() const {
  return new Array(*this);
}

Array::~Array() {
  delete index_type;
  delete element_type;
}

void Array::generate(std::ostream &out) const {
  out << "ArrayBase<" << *index_type << "," << *element_type << ">";
}

size_t Array::size() const {
  size_t s;
  size_t i = index_type->size();
  size_t e = element_type->size();
  if (__builtin_mul_overflow(i, e, &s))
    throw RumurError("overflow in calculating size of array", loc);
  return s;
}

TypeExprID::TypeExprID(const std::string &name_, TypeExpr *referent_,
  const location &loc_):
  TypeExpr(loc_), name(name_), referent(referent_) { }

TypeExprID::TypeExprID(const TypeExprID &other):
  TypeExpr(other.loc), name(other.name), referent(other.referent->clone()) { }

TypeExprID &TypeExprID::operator=(TypeExprID other) {
  swap(*this, other);
  return *this;
}

void swap(TypeExprID &x, TypeExprID &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.name, y.name);
  swap(x.referent, y.referent);
}

TypeExprID *TypeExprID::clone() const {
  return new TypeExprID(*this);
}

TypeExprID::~TypeExprID() {
  delete referent;
}

void TypeExprID::generate(std::ostream &out) const {
  out << name;
}

size_t TypeExprID::size() const {
  return referent->size();
}

}
