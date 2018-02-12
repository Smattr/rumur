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

const TypeExpr *TypeExpr::resolve() const {
  return this;
}

Range::Range(Expr *min_, Expr *max_, const location &loc_):
  TypeExpr(loc_), min(min_), max(max_) {
  if (!min->constant())
    throw Error("lower bound of range is not a constant", min->loc);

  if (!max->constant())
    throw Error("upper bound of range is not a constant", max->loc);
}

Range::Range(const Range &other):
  TypeExpr(other), min(other.min->clone()), max(other.max->clone()) {
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

Range::~Range() {
  delete min;
  delete max;
}

void Range::generate(std::ostream &out) const {
  int64_t lb = min->constant_fold();
  int64_t ub = max->constant_fold();
  out << "Range<" << lb << ", " << ub << ">";
}

size_t Range::width() const {
  int64_t lb = min->constant_fold();
  int64_t ub = max->constant_fold();
  uint64_t range;
  if (__builtin_sub_overflow(ub, lb, &range))
    throw Error("overflow in calculating width of range", loc);
  return bits_for(range);
}

size_t Range::count() const {
  int64_t lb = min->constant_fold();
  int64_t ub = max->constant_fold();
  size_t range;
  if (__builtin_sub_overflow(ub, lb, &range) ||
      __builtin_add_overflow(range, 1, &range))
    throw Error("overflow in calculating count of range", loc);
  return range;
}

bool Range::operator==(const Node &other) const {
  if (auto o = dynamic_cast<const Range*>(&other))
    return min->constant_fold() == o->min->constant_fold() &&
           max->constant_fold() == o->max->constant_fold();

  if (auto o = dynamic_cast<const TypeExprID*>(&other))
    return *this == *o->referent;

  return false;
}

bool Range::is_simple() const {
  return true;
}

Enum::Enum(const std::vector<std::pair<std::string, location>> &&members_, const location &loc_):
  TypeExpr(loc_), members(members_) {
}

Enum *Enum::clone() const {
  return new Enum(*this);
}

void Enum::generate(std::ostream &out) const {
  out << "Enum<";
  bool first = true;
  for (const std::pair<std::string, location> &m : members) {
    if (!first)
      out << ", ','";
    for (char c : m.first) {
      if (!first)
        out << ",";
      out << "'" << c << "'";
      first = false;
    }
  }
  out << ">";
}

size_t Enum::width() const {
  return bits_for(members.size());
}

size_t Enum::count() const {
  return members.size();
}

bool Enum::operator==(const Node &other) const {
  // FIXME: ignore location of member definitions in the following comparison
  if (auto o = dynamic_cast<const Enum*>(&other))
    return members == o->members;

  if (auto o = dynamic_cast<const TypeExprID*>(&other))
    return *this == *o->referent;

  return false;
}

bool Enum::is_simple() const {
  return true;
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

size_t Record::width() const {
  size_t s = 0;
  for (const VarDecl *v : fields) {
    size_t v_size = v->type->width();
    if (__builtin_add_overflow(s, v_size, &s))
      throw Error("overflow in calculating width of record", loc);
  }
  return s;
}

size_t Record::count() const {
  size_t s = 1;
  for (const VarDecl *v : fields) {
    if (__builtin_mul_overflow(s, v->type->count(), &s))
      throw Error("overflow in calculating count of record", loc);
  }
  return s;
}

bool Record::operator==(const Node &other) const {
  if (auto o = dynamic_cast<const Record*>(&other)) {
    for (auto it = fields.begin(), it2 = o->fields.begin(); ; it++, it2++) {
      if (it == fields.end())
        return it2 == o->fields.end();
      if (it2 == o->fields.end())
        return false;
      if (**it != **it2)
        return false;
    }
  }

  if (auto o = dynamic_cast<const TypeExprID*>(&other))
    return *this == *o->referent;

  return false;
}

Array::Array(TypeExpr *index_type_, TypeExpr *element_type_, const location &loc_):
  TypeExpr(loc_), index_type(index_type_), element_type(element_type_) {
  if (!index_type->is_simple())
    throw Error("array indices must be simple types", loc);
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
  out << "Array<" << *index_type << ", " << *element_type << ">";
}

size_t Array::width() const {
  size_t s;
  size_t i = index_type->count();
  size_t e = element_type->width();
  if (__builtin_mul_overflow(i, e, &s))
    throw Error("overflow in calculating width of array", loc);
  return s;
}

size_t Array::count() const {
  size_t s = 1;
  for (size_t i = 0; i < index_type->count(); i++) {
    if (__builtin_mul_overflow(s, element_type->count(), &s))
      throw Error("overflow in calculating count of array", loc);
  }
  return s;
}

bool Array::operator==(const Node &other) const {
  if (auto o = dynamic_cast<const Array*>(&other))
    return *index_type == *o->index_type
        && *element_type == *o->element_type;

  if (auto o = dynamic_cast<const TypeExprID*>(&other))
    return *this == *o->referent;

  return false;
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
  out << "ru_u_" << name;
}

size_t TypeExprID::width() const {
  return referent->width();
}

size_t TypeExprID::count() const {
  return referent->count();
}

bool TypeExprID::operator==(const Node &other) const {
  return *this->referent == other;
}

bool TypeExprID::is_simple() const {
  return referent->is_simple();
}

const TypeExpr *TypeExprID::resolve() const {
  return referent->resolve();
}

}
