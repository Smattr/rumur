#include <cassert>
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

std::string TypeExpr::lower_bound() const {
  throw Error("complex types do not have valid lower bounds", loc);
}

std::string TypeExpr::upper_bound() const {
  throw Error("complex types do not have valid upper bounds", loc);
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

size_t Range::width() const {
  int64_t lb = min->constant_fold();
  int64_t ub = max->constant_fold();
  uint64_t range;
  if (__builtin_sub_overflow(ub, lb, &range) ||
      __builtin_add_overflow(range, 1, &range))
    throw Error("overflow in calculating width of range", loc);
  return bits_for(range);
}

size_t Range::count() const {
  int64_t lb = min->constant_fold();
  int64_t ub = max->constant_fold();
  size_t range;
  if (__builtin_sub_overflow(ub, lb, &range) ||
      __builtin_add_overflow(range, 2, &range))
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

std::string Range::lower_bound() const {
  return "VALUE_C(" + std::to_string(min->constant_fold()) + ")";
}

std::string Range::upper_bound() const {
  return "VALUE_C(" + std::to_string(max->constant_fold()) + ")";
}

void Range::generate_print(std::ostream &out, std::string const &prefix,
  size_t preceding_offset) const {

  std::string const lb = lower_bound();
  std::string const ub = upper_bound();

  out
    << "{\n"
    << "  fprintf(stderr, \"" << prefix << ": \");\n"
    << "  value_t v = handle_read_raw((struct handle){ .base = "
      << "(uint8_t*)s->data, .offset = SIZE_C(" << preceding_offset << ") });\n"
    << "  if (v == 0) {\n"
    << "    fprintf(stderr, \"undefined\\n\");\n"
    << "  } else {\n"
    << "    fprintf(stderr, \"%\" PRIVAL \"\\n\", decode_value(" << lb << ", "
      << ub << ", v));\n"
    << "  }\n"
    << "}\n";
}

Scalarset::Scalarset(Expr *bound_, const location &loc_):
  TypeExpr(loc_), bound(bound_) {

  if (!bound->constant())
    throw Error("bound of scalarset is not a constant", bound->loc);

  if (bound->constant_fold() <= 0)
    throw Error("bound of scalarset is not positive", bound->loc);
}

Scalarset::Scalarset(const Scalarset &other):
  TypeExpr(other), bound(other.bound->clone()) { }

Scalarset &Scalarset::operator=(Scalarset other) {
  swap(*this, other);
  return *this;
}

void swap(Scalarset &x, Scalarset &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.bound, y.bound);
}

Scalarset *Scalarset::clone() const {
  return new Scalarset(*this);
}

Scalarset::~Scalarset() {
  delete bound;
}

size_t Scalarset::width() const {
  int64_t b = bound->constant_fold();
  assert(b > 0 && "non-positive bound for scalarset");

  uint64_t range;
  if (__builtin_add_overflow(b, 1, &range))
    throw Error("overflow in calculating width of scalarset", loc);

  return bits_for(range);
}

size_t Scalarset::count() const {
  int64_t b = bound->constant_fold();
  assert(b > 0 && "non-positive bound for scalarset");

  uint64_t range;
  if (__builtin_add_overflow(b, 2, &range))
    throw Error("overflow in calculating count of scalarset", loc);

  return range;
}

bool Scalarset::operator==(const Node &other) const {
  if (auto o = dynamic_cast<const Scalarset*>(&other))
    return bound->constant_fold() == o->bound->constant_fold();

  if (auto o = dynamic_cast<const TypeExprID*>(&other))
    return *this == *o->referent;

  return false;
}

bool Scalarset::is_simple() const {
  return true;
}

std::string Scalarset::lower_bound() const {
  return "VALUE_C(0)";
}

std::string Scalarset::upper_bound() const {
  return "VALUE_C(" + std::to_string(bound->constant_fold() - 1) + ")";
}

void Scalarset::generate_print(std::ostream &out, std::string const &prefix,
  size_t preceding_offset) const {

  out
    << "{\n"
    << "  fprintf(stderr, \"" << prefix << ": \");\n"
    << "  value_t v = handle_read_raw((struct handle){ .base = "
      << "(uint8_t*)s->data, .offset = SIZE_C(" << preceding_offset << ") });\n"
    << "  if (v == 0) {\n"
    << "    fprintf(stderr, \"undefined\\n\");\n"
    << "  } else {\n"
    << "    fprintf(stderr, \"%\" PRIVAL \"\\n\", v - 1);\n"
    << "  }\n"
    << "}\n";
}

Enum::Enum(const std::vector<std::pair<std::string, location>> &&members_, const location &loc_):
  TypeExpr(loc_), members(members_) {
}

Enum *Enum::clone() const {
  return new Enum(*this);
}

size_t Enum::width() const {
  return bits_for(members.size());
}

size_t Enum::count() const {
  size_t r;
  if (__builtin_add_overflow(members.size(), 1, &r))
    throw Error("overflow in calculating count of enum", loc);
  return r;
}

bool Enum::operator==(const Node &other) const {
  if (auto o = dynamic_cast<const Enum*>(&other)) {
    for (auto it = members.begin(), it2 = o->members.begin(); ; it++, it2++) {
      if (it == members.end()) {
        if (it2 != o->members.end())
          return false;
        break;
      }
      if (it2 == o->members.end())
        return false;
      if ((*it).first != (*it2).first)
        return false;
    }
    return true;
  }

  if (auto o = dynamic_cast<const TypeExprID*>(&other))
    return *this == *o->referent;

  return false;
}

bool Enum::is_simple() const {
  return true;
}

std::string Enum::lower_bound() const {
  return "0";
}

std::string Enum::upper_bound() const {
  return "VALUE_C("
    + std::to_string(members.size() == 0 ? 0 : members.size() - 1) + ")";
}

void Enum::generate_print(std::ostream &out, std::string const &prefix,
  size_t preceding_offset) const {

  out
    << "{\n"
    << "  fprintf(stderr, \"" << prefix << ": \");\n"
    << "  value_t v = handle_read_raw((struct handle){ .base = "
      << "(uint8_t*)s->data, .offset = SIZE_C(" << preceding_offset << ") });\n"
    << "  if (v == 0) {\n"
    << "    fprintf(stderr, \"undefined\\n\");\n";
  size_t i = 0;
  for (std::pair<std::string, location> const &m : members) {
    out
      << "  } else if (v == VALUE_C(" << i << ")) {\n"
      << "    fprintf(stderr, \"%s\\n\", \"" << m.first << "\");\n";
    i++;
  }
  out
    << "  } else {\n"
    << "    fprintf(stderr, \"ILLEGAL VALUE\\n\");\n"
    << "  }\n"
    << "}\n";
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

void Record::generate_print(std::ostream &out, std::string const &prefix,
  size_t preceding_offset) const {

  for (VarDecl const *f : fields) {
    f->generate_print(out, prefix + ".", preceding_offset);
    preceding_offset += f->width();
  }
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

void Array::generate_print(std::ostream &out, std::string const &prefix,
  size_t preceding_offset) const {

  TypeExpr const *t = index_type->resolve();

  if (auto r = dynamic_cast<Range const*>(t)) {

    int64_t lb = r->min->constant_fold();
    int64_t ub = r->max->constant_fold();

    // FIXME: Unrolling this loop at generation time is not great if the index
    // type is big. E.g. if someone has an 'array [0..10000] of ...' this is
    // going to generate quite unpleasant code.
    for (int64_t i = lb; i <= ub; i++) {

      element_type->generate_print(out, prefix + "[" + std::to_string(i) + "]",
        preceding_offset);
      preceding_offset += element_type->width();

      if (ub == INT64_MAX && i == INT64_MAX)
        break;
    }

    return;
  }

  if (auto s = dynamic_cast<Scalarset const*>(t)) {

    int64_t b = s->bound->constant_fold();

    for (int64_t i = 0; i < b; i++) {
      element_type->generate_print(out, prefix + "[" + std::to_string(i) + "]",
        preceding_offset);
      preceding_offset += element_type->width();
    }

    return;
  }

  if (auto e = dynamic_cast<Enum const*>(t)) {

    for (std::pair<std::string, location> const &m : e->members) {
      element_type->generate_print(out, prefix + "[" + m.first + "]",
        preceding_offset);
      preceding_offset += element_type->width();
    }

    return;
  }

  assert(!"non-range, non-enum used as array index");
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

std::string TypeExprID::lower_bound() const {
  return referent->lower_bound();
}

std::string TypeExprID::upper_bound() const {
  return referent->upper_bound();
}

void TypeExprID::generate_print(std::ostream &out, std::string const &prefix,
  size_t preceding_offset) const {
  referent->generate_print(out, prefix, preceding_offset);
}

}
