#include <cassert>
#include <cstdint>
#include <iostream>
#include "location.hh"
#include "macros.h"
#include <memory>
#include <optional>
#include <rumur/Boolean.h>
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/Indexer.h>
#include <rumur/TypeExpr.h>
#include <string>

using namespace rumur;
using namespace std;

bool Expr::is_arithmetic() const noexcept {

    // Is this a literal?
    if (type() == nullptr)
        return true;

    // Is this of a range type?
    if (dynamic_cast<const Range*>(type()) != nullptr)
        return true;

    return false;
}

static void expect_arithmetic(const shared_ptr<Expr> e) {
    if (!e->is_arithmetic())
        throw RumurError("expected arithmetic expression is not arithmetic",
          e->loc);
}

bool Expr::is_boolean() const noexcept {
    return type() == &Boolean;
}

static void expect_boolean(const shared_ptr<Expr> e) {
    if (!e->is_boolean())
        throw RumurError("expected boolean expression is not a boolean",
          e->loc);
}

Expr::~Expr() {
}

Ternary::Ternary(shared_ptr<Expr> cond, shared_ptr<Expr> lhs,
  shared_ptr<Expr> rhs, const location &loc, Indexer&) noexcept
  : Expr(loc), cond(cond), lhs(lhs), rhs(rhs) {
}

void Ternary::validate() const {
    cond->validate();
    lhs->validate();
    rhs->validate();

    expect_boolean(cond);

    // TODO: check lhs and rhs have the same type
}

bool Ternary::constant() const noexcept {
    return cond->constant() && lhs->constant() && rhs->constant();
}

const TypeExpr *Ternary::type() const noexcept {
    // TODO: assert lhs and rhs are compatible types.
    return lhs->type();
}

void Ternary::rvalue(ostream &out) const {
    out << "(";
    cond->rvalue(out);
    out << "?";
    lhs->rvalue(out);
    out << ":";
    rhs->rvalue(out);
    out << ")";
}

BinaryExpr::BinaryExpr(shared_ptr<Expr> lhs, shared_ptr<Expr> rhs,
  const location &loc, Indexer&) noexcept
  : Expr(loc), lhs(lhs), rhs(rhs) {
}

void BinaryExpr::validate() const {
    lhs->validate();
    rhs->validate();
}

bool BinaryExpr::constant() const noexcept {
    return lhs->constant() && rhs->constant();
}

void Implication::validate() const {
    BinaryExpr::validate();

    expect_boolean(lhs);
    expect_boolean(rhs);
}

const TypeExpr *Implication::type() const noexcept {
    return &Boolean;
}

void Or::validate() const {
    BinaryExpr::validate();

    expect_boolean(lhs);
    expect_boolean(rhs);
}

void Implication::rvalue(ostream &out) const {
    out << "(!";
    lhs->rvalue(out);
    out << "||";
    rhs->rvalue(out);
    out << ")";
}

const TypeExpr *Or::type() const noexcept {
    return &Boolean;
}

void Or::rvalue(ostream &out) const {
    out << "(";
    lhs->rvalue(out);
    out << "||";
    rhs->rvalue(out);
    out << ")";
}

void And::validate() const {
    BinaryExpr::validate();

    expect_boolean(lhs);
    expect_boolean(rhs);
}

const TypeExpr *And::type() const noexcept {
    return &Boolean;
}

void And::rvalue(ostream &out) const {
    out << "(";
    lhs->rvalue(out);
    out << "&&";
    rhs->rvalue(out);
    out << ")";
}

UnaryExpr::UnaryExpr(shared_ptr<Expr> rhs, const location &loc,
  Indexer&) noexcept
  : Expr(loc), rhs(rhs) {
}

void UnaryExpr::validate() const {
    rhs->validate();
}

bool UnaryExpr::constant() const noexcept {
    return rhs->constant();
}

void Not::validate() const {
    UnaryExpr::validate();

    expect_boolean(rhs);
}

const TypeExpr *Not::type() const noexcept {
    return &Boolean;
}

void Not::rvalue(ostream &out) const {
    out << "(!";
    rhs->rvalue(out);
    out << ")";
}

void Lt::validate() const {
    BinaryExpr::validate();

    expect_arithmetic(lhs);
    expect_arithmetic(rhs);
}

const TypeExpr *Lt::type() const noexcept {
    return &Boolean;
}

void Lt::rvalue(ostream &out) const {
    out << "(";
    lhs->rvalue(out);
    out << "<";
    rhs->rvalue(out);
    out << ")";
}

void Leq::validate() const {
    BinaryExpr::validate();

    expect_arithmetic(lhs);
    expect_arithmetic(rhs);
}

const TypeExpr *Leq::type() const noexcept {
    return &Boolean;
}

void Leq::rvalue(ostream &out) const {
    out << "(";
    lhs->rvalue(out);
    out << "<=";
    rhs->rvalue(out);
    out << ")";
}

void Gt::validate() const {
    BinaryExpr::validate();

    expect_arithmetic(lhs);
    expect_arithmetic(rhs);
}

const TypeExpr *Gt::type() const noexcept {
    return &Boolean;
}

void Gt::rvalue(ostream &out) const {
    out << "(";
    lhs->rvalue(out);
    out << ">";
    rhs->rvalue(out);
    out << ")";
}

void Geq::validate() const {
    BinaryExpr::validate();

    expect_arithmetic(lhs);
    expect_arithmetic(rhs);
}

const TypeExpr *Geq::type() const noexcept {
    return &Boolean;
}

void Geq::rvalue(ostream &out) const {
    out << "(";
    lhs->rvalue(out);
    out << ">=";
    rhs->rvalue(out);
    out << ")";
}

void Eq::validate() const {
    BinaryExpr::validate();

    if (lhs->is_boolean()) {
        if (!rhs->is_boolean()) {
            throw RumurError("left hand side of comparison is boolean but "
              "right hand side is not", loc);
        }
    } else if (lhs->is_arithmetic()) {
        if (!rhs->is_arithmetic()) {
            throw RumurError("left hand side of comparison is arithmetic but "
              "right hand side is not", loc);
        }
    }
    // TODO test other comparable pairs
}

const TypeExpr *Eq::type() const noexcept {
    return &Boolean;
}

void Eq::rvalue(ostream &out) const {
    out << "(";
    lhs->rvalue(out);
    out << "==";
    rhs->rvalue(out);
    out << ")";
}

void Neq::validate() const {
    BinaryExpr::validate();

    if (lhs->is_boolean()) {
        if (!rhs->is_boolean()) {
            throw RumurError("left hand side of comparison is boolean but "
              "right hand side is not", loc);
        }
    } else if (lhs->is_arithmetic()) {
        if (!rhs->is_arithmetic()) {
            throw RumurError("left hand side of comparison is arithmetic but "
              "right hand side is not", loc);
        }
    }
    // TODO test other comparable pairs
}

const TypeExpr *Neq::type() const noexcept {
    return &Boolean;
}

void Neq::rvalue(ostream &out) const {
    out << "(";
    lhs->rvalue(out);
    out << "!=";
    rhs->rvalue(out);
    out << ")";
}

void Add::validate() const {
    BinaryExpr::validate();

    expect_arithmetic(lhs);
    expect_arithmetic(rhs);
}

const TypeExpr *Add::type() const noexcept {
    return nullptr;
}

void Add::rvalue(ostream &out) const {
    out << "add(";
    lhs->rvalue(out);
    out << ",";
    rhs->rvalue(out);
    out << ")";
}

void Sub::validate() const {
    BinaryExpr::validate();

    expect_arithmetic(lhs);
    expect_arithmetic(rhs);
}

const TypeExpr *Sub::type() const noexcept {
    return nullptr;
}

void Sub::rvalue(ostream &out) const {
    out << "sub(";
    lhs->rvalue(out);
    out << ",";
    rhs->rvalue(out);
    out << ")";
}

void Negative::validate() const {
    rhs->validate();
    expect_arithmetic(rhs);
}

const TypeExpr *Negative::type() const noexcept {
    return rhs->type();
}

void Negative::rvalue(ostream &out) const {
    out << "negate(";
    rhs->rvalue(out);
    out << ")";
}

void Mul::validate() const {
    BinaryExpr::validate();

    expect_arithmetic(lhs);
    expect_arithmetic(rhs);
}

const TypeExpr *Mul::type() const noexcept {
    return nullptr;
}

void Mul::rvalue(ostream &out) const {
    out << "mul(";
    lhs->rvalue(out);
    out << ",";
    rhs->rvalue(out);
    out << ")";
}

void Div::validate() const {
    BinaryExpr::validate();

    expect_arithmetic(lhs);
    expect_arithmetic(rhs);
}

const TypeExpr *Div::type() const noexcept {
    return nullptr;
}

void Div::rvalue(ostream &out) const {
    out << "divide(";
    lhs->rvalue(out);
    out << ",";
    rhs->rvalue(out);
    out << ")";
}

void Mod::validate() const {
    BinaryExpr::validate();

    expect_arithmetic(lhs);
    expect_arithmetic(rhs);
}

const TypeExpr *Mod::type() const noexcept {
    return nullptr;
}

void Mod::rvalue(ostream &out) const {
    out << "mod(";
    lhs->rvalue(out);
    out << ",";
    rhs->rvalue(out);
    out << ")";
}

/* Cheap trick: this destructor is pure virtual in the class declaration, making
 * the class abstract.
 */
Lvalue::~Lvalue() {
}

ExprID::ExprID(const string &id, shared_ptr<Expr> value,
  const TypeExpr *type_of, const location &loc, Indexer&)
  : Lvalue(loc), id(id), value(value), type_of(type_of) {
}

bool ExprID::constant() const noexcept {
    return value->constant();
}

void ExprID::validate() const {
    // FIXME: Is this relevant? An ExprID is just referencing another expression
    // we've probably already checked.
    value->validate();
}

const TypeExpr *ExprID::type() const noexcept {
    return type_of;
}

void ExprID::rvalue(ostream &out) const {
    out << "model_" << id;
}

void ExprID::lvalue(ostream &out) const {
    auto l = dynamic_pointer_cast<Lvalue>(value);
    assert(l != nullptr);
    l->lvalue(out);
}

Var::Var(shared_ptr<VarDecl> decl, const location &loc, Indexer&)
  : Lvalue(loc), decl(decl) {
}

bool Var::constant() const noexcept {
    return false;
}

const TypeExpr *Var::type() const noexcept {
    return decl->type.get();
}

void Var::rvalue(ostream &out) const {
    if (decl->local) {
        out << "model_" << decl->name;
    } else {
        out << "state_read_" << decl->name << "(s)";
    }
}

void Var::lvalue(ostream &out) const {
    if (decl->local) {
        out << "model_" << decl->name;
    } else {
        out << "state_reference_" << decl->name << "(s)";
    }
}

Field::Field(shared_ptr<Lvalue> record, const string &field, const location &loc,
  Indexer&)
  : Lvalue(loc), record(record), field(field) {
}

bool Field::constant() const noexcept {
    return record->constant();
}

void Field::rvalue(ostream &out) const {
    const TypeExpr *t = record->type();
    assert(t != nullptr && "root of field reference with no type");
    auto r = dynamic_cast<const Record*>(t);
    assert(r != nullptr && "root of field reference with non-record type");

    shared_ptr<SimpleTypeExpr> f = nullptr;
    for (shared_ptr<VarDecl> v : r->fields) {
        if (v->name == field) {
            f = dynamic_pointer_cast<SimpleTypeExpr>(v->type);
            break;
        }
    }

    if (f != nullptr) {
        /* HACK: if this field has a simple type, "unwrap" it into a bare
         * ``int64_t``. This way callers of this can rely on getting back a
         * reference in the case of an aggregate type and an ``int64_t`` in the
         * case of a simple type.
         * FIXME: it is not immediately obvious from the above comment *why*
         * this is desirable.
         */
        f->reader(out);
        out << "(";
    }

    lvalue(out);

    if (f != nullptr) {
        out << ")";
    }
}

const TypeExpr *Field::type() const noexcept {
    // TODO
    return nullptr;
}

void Field::lvalue(ostream &out) const {
    const TypeExpr *t = record->type();
    assert(t != nullptr && "root of field reference with no type");
    auto r = dynamic_cast<const Record*>(t);
    assert(r != nullptr && "root of field reference with non-record type");
    r->field_referencer(out, field);
    out << "(";
    record->lvalue(out);
    out << ")";
}

Element::Element(shared_ptr<Lvalue> array, shared_ptr<Expr> index,
  const location &loc, Indexer&)
  : Lvalue(loc), array(array), index(index) {
}

bool Element::constant() const noexcept {
    return array->constant() && index->constant();
}

const TypeExpr *Element::type() const noexcept {
    // TODO
    return nullptr;
}

void Element::rvalue(ostream &out) const {
    const TypeExpr *t = array->type();
    assert(t != nullptr && "root of element reference with no type");
    auto a = dynamic_cast<const Array*>(t);
    assert(a != nullptr && "root of element reference with non-array type");
    auto e = dynamic_pointer_cast<SimpleTypeExpr>(a->element_type);

    if (e != nullptr) {
        /* HACK: We do the same trick as Field::rvalue above to emit either a
         * reference or ``int64_t`` based on the element type of this array.
         */
        e->reader(out);
        out << "(";
    }

    lvalue(out);

    if (e != nullptr) {
        out << ")";
    }
}

void Element::lvalue(ostream &out) const {
    const TypeExpr *t = array->type();
    assert(t != nullptr && "root of element reference with no type");
    auto a = dynamic_cast<const Array*>(t);
    assert(a != nullptr && "root of element reference with non-array type");
    a->element_referencer(out);
    out << "(";
    array->lvalue(out);
    out << ",";
    index->rvalue(out);
    out << ")";
}

Quantifier::Quantifier(const string &name, shared_ptr<TypeExpr> type,
  const location &loc, Indexer &indexer)
  : Node(loc), var(make_shared<VarDecl>(name, type, loc, indexer)) {
}

Quantifier::Quantifier(const string &name, shared_ptr<Expr> from,
  shared_ptr<Expr> to, const location &loc, Indexer& indexer)
  : Quantifier(loc, name, from, to, {}, indexer) {
}

Quantifier::Quantifier(const string &name, shared_ptr<Expr> from,
  shared_ptr<Expr> to, shared_ptr<Expr> step, const location &loc,
  Indexer &indexer)
  : Quantifier(loc, name, from, to, step, indexer) {
}

Quantifier::Quantifier(const location &loc, const string &name,
  shared_ptr<Expr> from, shared_ptr<Expr> to, optional<shared_ptr<Expr>> step,
  Indexer &indexer)
  : Node(loc),
    var(make_shared<VarDecl>(name, make_shared<Range>(from, to, loc, indexer), loc, indexer)),
    step(step) {
}

Exists::Exists(shared_ptr<Quantifier> quantifier, shared_ptr<Expr> expr,
  const location &loc, Indexer&)
  : Expr(loc), quantifier(quantifier), expr(expr) {
}

bool Exists::constant() const noexcept {
    return expr->constant();
}

const TypeExpr *Exists::type() const noexcept {
    return &Boolean;
}

void Exists::rvalue(ostream &out) const {
    out << "({bool r=false;for(int64_t model_" << quantifier->var->name << "=";
    quantifier->var->type->generate_min(out);
    out << ";;model_" << quantifier->var->name << "=add(model_"
      << quantifier->var->name << ",";
    if (quantifier->step.has_value()) {
        quantifier->step.value()->rvalue(out);
    } else {
        out << "1";
    }
    out << ")){r|=";
    expr->rvalue(out);
    out << ";if(r||" << quantifier->var->name << "==";
    quantifier->var->type->generate_max(out);
    out << "){break;}}r;})";
}

Forall::Forall(shared_ptr<Quantifier> quantifier, shared_ptr<Expr> expr,
  const location &loc, Indexer&)
  : Expr(loc), quantifier(quantifier), expr(expr) {
}

bool Forall::constant() const noexcept {
    return expr->constant();
}

const TypeExpr *Forall::type() const noexcept {
    return &Boolean;
}

void Forall::rvalue(ostream &out) const {
    out << "({bool r=true;for(int64_t model_" << quantifier->var->name << "=";
    quantifier->var->type->generate_min(out);
    out << ";;model_" << quantifier->var->name << "=add(model_"
      << quantifier->var->name << ",";
    if (quantifier->step.has_value()) {
        quantifier->step.value()->rvalue(out);
    } else {
        out << "1";
    }
    out << ")){r&=";
    expr->rvalue(out);
    out << ";if(!r||" << quantifier->var->name << "==";
    quantifier->var->type->generate_max(out);
    out << "){break;}}r;})";
}
