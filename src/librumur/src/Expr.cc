#include <cassert>
#include <cstdint>
#include <iostream>
#include "location.hh"
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

void Ternary::generate_read(ostream &out) const {
    out << "(";
    cond->generate_read(out);
    out << "?";
    lhs->generate_read(out);
    out << ":";
    rhs->generate_read(out);
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

void Implication::generate_read(ostream &out) const {
    out << "(!";
    lhs->generate_read(out);
    out << "||";
    rhs->generate_read(out);
    out << ")";
}

const TypeExpr *Or::type() const noexcept {
    return &Boolean;
}

void Or::generate_read(ostream &out) const {
    out << "(";
    lhs->generate_read(out);
    out << "||";
    rhs->generate_read(out);
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

void And::generate_read(ostream &out) const {
    out << "(";
    lhs->generate_read(out);
    out << "&&";
    rhs->generate_read(out);
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

void Not::generate_read(ostream &out) const {
    out << "(!";
    rhs->generate_read(out);
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

void Lt::generate_read(ostream &out) const {
    out << "(";
    lhs->generate_read(out);
    out << "<";
    rhs->generate_read(out);
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

void Leq::generate_read(ostream &out) const {
    out << "(";
    lhs->generate_read(out);
    out << "<=";
    rhs->generate_read(out);
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

void Gt::generate_read(ostream &out) const {
    out << "(";
    lhs->generate_read(out);
    out << ">";
    rhs->generate_read(out);
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

void Geq::generate_read(ostream &out) const {
    out << "(";
    lhs->generate_read(out);
    out << ">=";
    rhs->generate_read(out);
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

void Eq::generate_read(ostream &out) const {
    out << "(";
    lhs->generate_read(out);
    out << "==";
    rhs->generate_read(out);
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

void Neq::generate_read(ostream &out) const {
    out << "(";
    lhs->generate_read(out);
    out << "!=";
    rhs->generate_read(out);
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

void Add::generate_read(ostream &out) const {
    out << "add(";
    lhs->generate_read(out);
    out << ",";
    rhs->generate_read(out);
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

void Sub::generate_read(ostream &out) const {
    out << "sub(";
    lhs->generate_read(out);
    out << ",";
    rhs->generate_read(out);
    out << ")";
}

void Negative::validate() const {
    rhs->validate();
    expect_arithmetic(rhs);
}

const TypeExpr *Negative::type() const noexcept {
    return rhs->type();
}

void Negative::generate_read(ostream &out) const {
    out << "negate(";
    rhs->generate_read(out);
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

void Mul::generate_read(ostream &out) const {
    out << "mul(";
    lhs->generate_read(out);
    out << ",";
    rhs->generate_read(out);
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

void Div::generate_read(ostream &out) const {
    out << "divide(";
    lhs->generate_read(out);
    out << ",";
    rhs->generate_read(out);
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

void Mod::generate_read(ostream &out) const {
    out << "mod(";
    lhs->generate_read(out);
    out << ",";
    rhs->generate_read(out);
    out << ")";
}

ExprID::ExprID(const string &id, shared_ptr<Expr> value,
  const TypeExpr *type_of, const location &loc, Indexer&)
  : Expr(loc), id(id), value(value), type_of(type_of) {
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

void ExprID::generate_read(ostream &out) const {
    out << "model_" << id;
}

Var::Var(shared_ptr<VarDecl> decl, const location &loc, Indexer&)
  : Expr(loc), decl(decl) {
}

bool Var::constant() const noexcept {
    return false;
}

const TypeExpr *Var::type() const noexcept {
    return decl->type.get();
}

void Var::generate_read(ostream &out) const {
    if (decl->local) {
        out << "model_" << decl->name;
    } else {
        out << "state_read_" << decl->name << "(s)";
    }
}

Field::Field(shared_ptr<Expr> record, const string &field, const location &loc,
  Indexer&)
  : Expr(loc), record(record), field(field) {
}

bool Field::constant() const noexcept {
    return record->constant();
}

void Field::generate_read(ostream &out) const {
    const TypeExpr *t = record->type();
    assert(t != nullptr);
    out << t->field_reader(field) << "(";

    record->generate_read(out);
    out << ")";
}

const TypeExpr *Field::type() const noexcept {
    // TODO
    return nullptr;
}

Element::Element(shared_ptr<Expr> array, shared_ptr<Expr> index,
  const location &loc, Indexer&)
  : Expr(loc), array(array), index(index) {
}

bool Element::constant() const noexcept {
    return array->constant() && index->constant();
}

const TypeExpr *Element::type() const noexcept {
    // TODO
    return nullptr;
}

void Element::generate_read(ostream &out) const {
    const TypeExpr *t = array->type();
    assert(t != nullptr);
    out << t->element_reader() << "(";

    array->generate_read(out);
    out << ",";
    index->generate_read(out);
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

void Exists::generate_read(ostream &out) const {
    out << "({bool r=false;for(int64_t model_" << quantifier->var->name << "=";
    quantifier->var->type->generate_min(out);
    out << ";;model_" << quantifier->var->name << "=add(model_"
      << quantifier->var->name << ",";
    if (quantifier->step.has_value()) {
        quantifier->step.value()->generate_read(out);
    } else {
        out << "1";
    }
    out << ")){r|=";
    expr->generate_read(out);
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

void Forall::generate_read(ostream &out) const {
    out << "({bool r=true;for(int64_t model_" << quantifier->var->name << "=";
    quantifier->var->type->generate_min(out);
    out << ";;model_" << quantifier->var->name << "=add(model_"
      << quantifier->var->name << ",";
    if (quantifier->step.has_value()) {
        quantifier->step.value()->generate_read(out);
    } else {
        out << "1";
    }
    out << ")){r&=";
    expr->generate_read(out);
    out << ";if(!r||" << quantifier->var->name << "==";
    quantifier->var->type->generate_max(out);
    out << "){break;}}r;})";
}
