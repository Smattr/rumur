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

void TypeExpr::generate_min(std::ostream&) const {
    throw RumurError("generate_min called on a complex type", loc);
}

void TypeExpr::generate_max(std::ostream&) const {
    throw RumurError("generate_max called on a complex type", loc);
}

std::string TypeExpr::field_reader(const std::string&) const {
    throw RumurError("field read of something that is not a record", loc);
}

std::string TypeExpr::field_writer(const std::string&) const {
    throw RumurError("field write of something that is not a record", loc);
}

std::string TypeExpr::element_reader() const {
    throw RumurError("element read of something that is not an array", loc);
}

std::string TypeExpr::element_writer() const {
    throw RumurError("element write of something that is not an array", loc);
}

SimpleTypeExpr::SimpleTypeExpr(const location &loc, Indexer &indexer)
  : TypeExpr(loc), index(indexer.new_index()) {
}

bool SimpleTypeExpr::is_simple() const {
    return true;
}

void SimpleTypeExpr::reader(std::ostream &out) const {
    out << "type_read_" << index;
}

void SimpleTypeExpr::writer(std::ostream &out) const {
    out << "type_write_" << index;
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

void Range::generate_min(std::ostream &out) const {
    min->rvalue(out);
}

void Range::generate_max(std::ostream &out) const {
    max->rvalue(out);
}

void Range::define(std::ostream &out) const {
    // Emit a type struct.
    out << "struct type_" << index << "{void*base;unsigned long offset;};";

    // Emit a reader for this type.
    out << "static int64_t ";
    reader(out);
    out << "(const struct context*context,const struct state*s,const struct type_" << index << " *t){ /* TODO */}";

    // Emit a writer for this type.
    out << "static void ";
    writer(out);
    out << "(const struct context*context,const struct state*s,const struct type_" << index << " *t,int64_t value){"
      "if(value<";
    generate_min(out);
    out << "){context->error(context,s,\"...\");}if(value>";
    generate_max(out);
    out << "){context->error(context,s,\"...\");} /* TODO */}";
}

Range::~Range() {
    delete min;
    delete max;
}

void Range::generate(std::ostream &out) const {
    out << "RangeBase<" << *min << "," << *max << ">";
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

void Enum::generate_min(std::ostream &out) const {
    out << "INT64_C(0)";
}

void Enum::generate_max(std::ostream &out) const {
    out << "INT64_C(" << members.size() << ")";
}

void Enum::define(std::ostream &) const {
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

void Record::field_referencer(std::ostream &out, const std::string &field) const {
    out << "make_reference_" << index << field;
}

std::string Record::field_reader(const std::string &field) const {
    unsigned long i = 0;
    for (const VarDecl *v : fields) {
        if (v->name == field) {
            return name + "_field_" + std::to_string(i) + "_read";
        }
        i++;
    }
    throw RumurError("attempted read of non-existent record field", loc);
}

std::string Record::field_writer(const std::string &field) const {
    unsigned long i = 0;
    for (const VarDecl *v : fields) {
        if (v->name == field) {
            return name + "_field_" + std::to_string(i) + "_write";
        }
        i++;
    }
    throw RumurError("attempted write of non-existent record field", loc);
}

void Record::define(std::ostream &) const {
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

void Array::element_referencer(std::ostream &out) const {
    out << "make_reference_" << index;
}

std::string Array::element_reader() const {
    return name + "_read";
}

std::string Array::element_writer() const {
    return name + "_write";
}

void Array::define(std::ostream &) const {
}

Array::~Array() {
    delete index_type;
    delete element_type;
}

void Array::generate(std::ostream &out) const {
    out << "ArrayBase<" << *index_type << "," << *element_type << ">";
}

}
