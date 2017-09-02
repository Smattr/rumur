#include <iostream>
#include <memory>
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/Indexer.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>
#include <vector>

using namespace rumur;
using namespace std;

bool TypeExpr::is_simple() const {
    return false;
}

void TypeExpr::generate_min(ostream&) const {
    throw RumurError("generate_min called on a complex type", loc);
}

void TypeExpr::generate_max(ostream&) const {
    throw RumurError("generate_max called on a complex type", loc);
}

string TypeExpr::field_reader(const string&) const {
    throw RumurError("field read of something that is not a record", loc);
}

string TypeExpr::field_writer(const string&) const {
    throw RumurError("field write of something that is not a record", loc);
}

string TypeExpr::element_reader() const {
    throw RumurError("element read of something that is not an array", loc);
}

string TypeExpr::element_writer() const {
    throw RumurError("element write of something that is not an array", loc);
}

bool SimpleTypeExpr::is_simple() const {
    return true;
}

Range::Range(shared_ptr<Expr> min, shared_ptr<Expr> max, const location &loc,
  Indexer&)
  : SimpleTypeExpr(loc), min(min), max(max) {
}

void Range::validate() const {
    if (!min->constant())
        throw RumurError("lower bound of range is not a constant", min->loc);

    if (!max->constant())
        throw RumurError("upper bound of range is not a constant", max->loc);
}

void Range::generate_min(ostream &out) const {
    min->generate_read(out);
}

void Range::generate_max(ostream &out) const {
    max->generate_read(out);
}

TypeExprID::TypeExprID(const string &id, shared_ptr<TypeExpr> value,
  const location &loc, Indexer&)
  : TypeExpr(loc), id(id), value(value) {
}

bool TypeExprID::is_simple() const {
    return value->is_simple();
}

void TypeExprID::generate_min(ostream &out) const {
    return value->generate_min(out);
}

void TypeExprID::generate_max(ostream &out) const {
    return value->generate_max(out);
}

string TypeExprID::field_reader(const string &field) const {
    return value->field_reader(field);
}

string TypeExprID::field_writer(const string &field) const {
    return value->field_writer(field);
}

string TypeExprID::element_reader() const {
    return value->element_reader();
}

string TypeExprID::element_writer() const {
    return value->element_writer();
}

Enum::Enum(const vector<pair<string, location>> &members, const location &loc,
  Indexer &indexer)
  : SimpleTypeExpr(loc) {

    for (auto [s, l] : members) {

        // Assign the enum member a numerical value
        auto n = make_shared<Number>(this->members.size(), l, indexer);

        // Construct an expression for it
        auto e = make_shared<ExprID>(s, n, this, l, indexer);
        this->members.emplace_back(e);

    }
}

void Enum::generate_min(ostream &out) const {
    out << "INT64_C(0)";
}

void Enum::generate_max(ostream &out) const {
    out << "INT64_C(" << members.size() << ")";
}

Record::Record(vector<shared_ptr<VarDecl>> &&fields, const location &loc,
  Indexer &indexer)
  : TypeExpr(loc), fields(fields), index(indexer.new_index()) {
}

void Record::field_referencer(ostream &out, const string &field) const {
    out << "make_reference_" << index << field;
}

string Record::field_reader(const string &field) const {
    unsigned long i = 0;
    for (const shared_ptr<VarDecl> v : fields) {
        if (v->name == field) {
            return name + "_field_" + to_string(i) + "_read";
        }
        i++;
    }
    throw RumurError("attempted read of non-existent record field", loc);
}

string Record::field_writer(const string &field) const {
    unsigned long i = 0;
    for (const shared_ptr<VarDecl> v : fields) {
        if (v->name == field) {
            return name + "_field_" + to_string(i) + "_write";
        }
        i++;
    }
    throw RumurError("attempted write of non-existent record field", loc);
}

Array::Array(shared_ptr<TypeExpr> index_type_,
  shared_ptr<TypeExpr> element_type_, const location &loc_,
  Indexer &indexer)
  : TypeExpr(loc_), index_type(index_type_), element_type(element_type_),
    index(indexer.new_index()) {
}

void Array::element_referencer(ostream &out) const {
    out << "make_reference_" << index;
}

string Array::element_reader() const {
    return name + "_read";
}

string Array::element_writer() const {
    return name + "_write";
}
