#include <iostream>
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/Indexer.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>
#include <vector>

namespace rumur {

bool TypeExpr::is_simple() const {
    return false;
}

SimpleTypeExpr::SimpleTypeExpr(const location &loc, Indexer &indexer)
  : TypeExpr(loc), index(indexer.new_index()) {
}

bool SimpleTypeExpr::is_simple() const {
    return true;
}

Range::Range(Expr *min, Expr *max, const location &loc, Indexer &indexer)
  : SimpleTypeExpr(loc, indexer), min(min), max(max) {
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
    swap(x.index, y.index);
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

Enum::Enum(const std::vector<std::pair<std::string, location>> &members, const location &loc,
  Indexer &indexer)
  : SimpleTypeExpr(loc, indexer) {

    for (const std::pair<std::string, location> &m : members) {

        // Assign the enum member a numerical value
        auto n = new Number(this->members.size(), m.second, indexer);

        // Construct an expression for it
        this->members.emplace_back(m.first, n, this, m.second, indexer);

    }
}

Enum *Enum::clone() const {
    return new Enum(*this);
}

void Enum::generate(std::ostream &out) const {
    out << "EnumBase<";
    bool first = true;
    for (const ExprID &m : members) {
        if (!first)
            out << ",";
        out << "\"" << m.id << "\"";
        first = false;
    }
    out << ">";
}

Record::Record(std::vector<VarDecl*> &&fields, const location &loc,
  Indexer &indexer)
  : TypeExpr(loc), fields(fields), index(indexer.new_index()) {
}

Record::Record(const Record &other):
  TypeExpr(other), index(other.index) {
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
    swap(x.name, y.name);
    swap(x.index, y.index);
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

Array::Array(TypeExpr *index_type_, TypeExpr *element_type_, const location &loc_,
  Indexer &indexer)
  : TypeExpr(loc_), index_type(index_type_), element_type(element_type_),
    index(indexer.new_index()) {
}

Array::Array(const Array &other):
  TypeExpr(other), index_type(other.index_type->clone()),
  element_type(other.element_type->clone()), index(other.index) {
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
    swap(x.name, y.name);
    swap(x.index, y.index);
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

}
