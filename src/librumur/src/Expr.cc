#include <cassert>
#include <cstdint>
#include <iostream>
#include "location.hh"
#include <rumur/Boolean.h>
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/Indexer.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>

namespace rumur {

bool Expr::is_arithmetic() const {

    // Is this a literal?
    if (type() == nullptr)
        return true;

    // Is this of a range type?
    if (dynamic_cast<const Range*>(type()) != nullptr)
        return true;

    return false;
}

static void expect_arithmetic(const Expr *e) {
    if (!e->is_arithmetic())
        throw RumurError("expected arithmetic expression is not arithmetic",
          e->loc);
}

bool Expr::is_boolean() const {
    return type() == &Boolean;
}

static void expect_boolean(const Expr *e) {
    if (!e->is_boolean())
        throw RumurError("expected boolean expression is not a boolean",
          e->loc);
}

Ternary::Ternary(Expr *cond, Expr *lhs, Expr *rhs, const location &loc,
    Indexer&):
    Expr(loc), cond(cond), lhs(lhs), rhs(rhs) {
}

Ternary::Ternary(const Ternary &other):
    Expr(other), cond(other.cond->clone()), lhs(other.lhs->clone()),
    rhs(other.rhs->clone()) {
}

Ternary &Ternary::operator=(Ternary other) {
    swap(*this, other);
    return *this;
}

void swap(Ternary &x, Ternary &y) noexcept {
    using std::swap;
    swap(x.loc, y.loc);
    swap(x.cond, y.cond);
    swap(x.lhs, y.lhs);
    swap(x.rhs, y.rhs);
}

Ternary *Ternary::clone() const {
    return new Ternary(*this);
}

Ternary::~Ternary() {
    delete cond;
    delete lhs;
    delete rhs;
}

void Ternary::validate() const {
    cond->validate();
    lhs->validate();
    rhs->validate();

    expect_boolean(cond);

    // TODO: check lhs and rhs have the same type
}

bool Ternary::constant() const {
    return cond->constant() && lhs->constant() && rhs->constant();
}

const TypeExpr *Ternary::type() const {
    // TODO: assert lhs and rhs are compatible types.
    return lhs->type();
}

void Ternary::rvalue(std::ostream &) const {
}

void Ternary::generate(std::ostream &out) const {
    out << "(" << *cond << "?" << *lhs << ":" << *rhs << ")";
}

BinaryExpr::BinaryExpr(Expr *lhs, Expr *rhs, const location &loc, Indexer&):
    Expr(loc), lhs(lhs), rhs(rhs) {
}

BinaryExpr::BinaryExpr(const BinaryExpr &other):
    Expr(other), lhs(other.lhs->clone()), rhs(other.rhs->clone()) {
}

void swap(BinaryExpr &x, BinaryExpr &y) noexcept {
    using std::swap;
    swap(x.loc, y.loc);
    swap(x.lhs, y.lhs);
    swap(x.rhs, y.rhs);
}

void BinaryExpr::validate() const {
    lhs->validate();
    rhs->validate();
}

bool BinaryExpr::constant() const {
    return lhs->constant() && rhs->constant();
}

BinaryExpr::~BinaryExpr() {
    delete lhs;
    delete rhs;
}

Implication &Implication::operator=(Implication other) {
    swap(*this, other);
    return *this;
}

Implication *Implication::clone() const {
    return new Implication(*this);
}

void Implication::validate() const {
    BinaryExpr::validate();

    expect_boolean(lhs);
    expect_boolean(rhs);
}

const TypeExpr *Implication::type() const {
    return &Boolean;
}

void Implication::rvalue(std::ostream &) const {
}

void Implication::generate(std::ostream &out) const {
    out << "(!" << *lhs << "||" << *rhs << ")";
}

Or &Or::operator=(Or other) {
    swap(*this, other);
    return *this;
}

Or *Or::clone() const {
    return new Or(*this);
}

void Or::validate() const {
    BinaryExpr::validate();

    expect_boolean(lhs);
    expect_boolean(rhs);
}

const TypeExpr *Or::type() const {
    return &Boolean;
}

void Or::rvalue(std::ostream &) const {
}

void Or::generate(std::ostream &out) const {
    out << "(" << *lhs << "||" << *rhs << ")";
}

And &And::operator=(And other) {
    swap(*this, other);
    return *this;
}

And *And::clone() const {
    return new And(*this);
}

void And::validate() const {
    BinaryExpr::validate();

    expect_boolean(lhs);
    expect_boolean(rhs);
}

const TypeExpr *And::type() const {
    return &Boolean;
}

void And::rvalue(std::ostream &) const {
}

void And::generate(std::ostream &out) const {
    out << "(" << *lhs << "&&" << *rhs << ")";
}

UnaryExpr::UnaryExpr(Expr *rhs, const location &loc, Indexer&):
    Expr(loc), rhs(rhs) {
}

UnaryExpr::UnaryExpr(const UnaryExpr &other):
    Expr(other), rhs(other.rhs->clone()) {
}

void swap(UnaryExpr &x, UnaryExpr &y) noexcept {
    using std::swap;
    swap(x.loc, y.loc);
    swap(x.rhs, y.rhs);
}

UnaryExpr::~UnaryExpr() {
    delete rhs;
}

void UnaryExpr::validate() const {
    rhs->validate();
}

bool UnaryExpr::constant() const {
    return rhs->constant();
}

Not &Not::operator=(Not other) {
    swap(*this, other);
    return *this;
}

Not *Not::clone() const {
    return new Not(*this);
}

void Not::validate() const {
    UnaryExpr::validate();

    expect_boolean(rhs);
}

const TypeExpr *Not::type() const {
    return &Boolean;
}

void Not::rvalue(std::ostream &) const {
}

void Not::generate(std::ostream &out) const {
    out << "(!" << *rhs << ")";
}

Lt &Lt::operator=(Lt other) {
    swap(*this, other);
    return *this;
}

Lt *Lt::clone() const {
    return new Lt(*this);
}

void Lt::validate() const {
    BinaryExpr::validate();

    expect_arithmetic(lhs);
    expect_arithmetic(rhs);
}

const TypeExpr *Lt::type() const {
    return &Boolean;
}

void Lt::rvalue(std::ostream &) const {
}

void Lt::generate(std::ostream &out) const {
    out << "(" << *lhs << "<" << *rhs << ")";
}

Leq &Leq::operator=(Leq other) {
    swap(*this, other);
    return *this;
}

Leq *Leq::clone() const {
    return new Leq(*this);
}

void Leq::validate() const {
    BinaryExpr::validate();

    expect_arithmetic(lhs);
    expect_arithmetic(rhs);
}

const TypeExpr *Leq::type() const {
    return &Boolean;
}

void Leq::rvalue(std::ostream &) const {
}

void Leq::generate(std::ostream &out) const {
    out << "(" << *lhs << "<=" << *rhs << ")";
}

Gt &Gt::operator=(Gt other) {
    swap(*this, other);
    return *this;
}

Gt *Gt::clone() const {
    return new Gt(*this);
}

void Gt::validate() const {
    BinaryExpr::validate();

    expect_arithmetic(lhs);
    expect_arithmetic(rhs);
}

const TypeExpr *Gt::type() const {
    return &Boolean;
}

void Gt::rvalue(std::ostream &) const {
}

void Gt::generate(std::ostream &out) const {
    out << "(" << *lhs << ">" << *rhs << ")";
}

Geq &Geq::operator=(Geq other) {
    swap(*this, other);
    return *this;
}

Geq *Geq::clone() const {
    return new Geq(*this);
}

void Geq::validate() const {
    BinaryExpr::validate();

    expect_arithmetic(lhs);
    expect_arithmetic(rhs);
}

const TypeExpr *Geq::type() const {
    return &Boolean;
}

void Geq::rvalue(std::ostream &) const {
}

void Geq::generate(std::ostream &out) const {
    out << "(" << *lhs << ">=" << *rhs << ")";
}

Eq &Eq::operator=(Eq other) {
    swap(*this, other);
    return *this;
}

Eq *Eq::clone() const {
    return new Eq(*this);
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

const TypeExpr *Eq::type() const {
    return &Boolean;
}

void Eq::rvalue(std::ostream &) const {
}

void Eq::generate(std::ostream &out) const {
    out << "(" << *lhs << "==" << *rhs << ")";
}

Neq &Neq::operator=(Neq other) {
    swap(*this, other);
    return *this;
}

Neq *Neq::clone() const {
    return new Neq(*this);
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

const TypeExpr *Neq::type() const {
    return &Boolean;
}

void Neq::rvalue(std::ostream &) const {
}

void Neq::generate(std::ostream &out) const {
    out << "(" << *lhs << "!=" << *rhs << ")";
}

Add &Add::operator=(Add other) {
    swap(*this, other);
    return *this;
}

Add *Add::clone() const {
    return new Add(*this);
}

void Add::validate() const {
    BinaryExpr::validate();

    expect_arithmetic(lhs);
    expect_arithmetic(rhs);
}

const TypeExpr *Add::type() const {
    return nullptr;
}

void Add::rvalue(std::ostream &) const {
}

void Add::generate(std::ostream &out) const {
    out << "(" << *lhs << "+" << *rhs << ")";
}

Sub &Sub::operator=(Sub other) {
    swap(*this, other);
    return *this;
}

Sub *Sub::clone() const {
    return new Sub(*this);
}

void Sub::validate() const {
    BinaryExpr::validate();

    expect_arithmetic(lhs);
    expect_arithmetic(rhs);
}

const TypeExpr *Sub::type() const {
    return nullptr;
}

void Sub::rvalue(std::ostream &) const {
}

void Sub::generate(std::ostream &out) const {
    out << "(" << *lhs << "-" << *rhs << ")";
}

void Negative::validate() const {
    rhs->validate();
    expect_arithmetic(rhs);
}

Negative &Negative::operator=(Negative other) {
    swap(*this, other);
    return *this;
}

Negative *Negative::clone() const {
    return new Negative(*this);
}

const TypeExpr *Negative::type() const {
    return rhs->type();
}

void Negative::rvalue(std::ostream &) const {
}

void Negative::generate(std::ostream &out) const {
    out << "(-" << *rhs << ")";
}

Mul &Mul::operator=(Mul other) {
    swap(*this, other);
    return *this;
}

Mul *Mul::clone() const {
    return new Mul(*this);
}

void Mul::validate() const {
    BinaryExpr::validate();

    expect_arithmetic(lhs);
    expect_arithmetic(rhs);
}

const TypeExpr *Mul::type() const {
    return nullptr;
}

void Mul::rvalue(std::ostream &) const {
}

void Mul::generate(std::ostream &out) const {
    out << "(" << *lhs << "*" << *rhs << ")";
}

Div &Div::operator=(Div other) {
    swap(*this, other);
    return *this;
}

Div *Div::clone() const {
    return new Div(*this);
}

void Div::validate() const {
    BinaryExpr::validate();

    expect_arithmetic(lhs);
    expect_arithmetic(rhs);
}

const TypeExpr *Div::type() const {
    return nullptr;
}

void Div::rvalue(std::ostream &) const {
}

void Div::generate(std::ostream &out) const {
    out << "(" << *lhs << "/" << *rhs << ")";
}

Mod &Mod::operator=(Mod other) {
    swap(*this, other);
    return *this;
}

Mod *Mod::clone() const {
    return new Mod(*this);
}

void Mod::validate() const {
    BinaryExpr::validate();

    expect_arithmetic(lhs);
    expect_arithmetic(rhs);
}

const TypeExpr *Mod::type() const {
    return nullptr;
}

void Mod::rvalue(std::ostream &) const {
}

void Mod::generate(std::ostream &out) const {
    out << "(" << *lhs << "%" << *rhs << ")";
}

/* Cheap trick: this destructor is pure virtual in the class declaration, making
 * the class abstract.
 */
Lvalue::~Lvalue() {
}

ExprID::ExprID(const std::string &id, const Expr *value,
  const TypeExpr *type_of, const location &loc, Indexer&)
  : Lvalue(loc), id(id), value(value->clone()), type_of(type_of) {
}

ExprID::ExprID(const ExprID &other):
  Lvalue(other), id(other.id), value(other.value->clone()), type_of(other.type_of) {
}

ExprID &ExprID::operator=(ExprID other) {
    swap(*this, other);
    return *this;
}

void swap(ExprID &x, ExprID &y) noexcept {
    using std::swap;
    swap(x.loc, y.loc);
    swap(x.id, y.id);
    swap(x.value, y.value);
    swap(x.type_of, y.type_of);
}

ExprID *ExprID::clone() const {
    return new ExprID(*this);
}

bool ExprID::constant() const {
    return value->constant();
}

void ExprID::validate() const {
    // FIXME: Is this relevant? An ExprID is just referencing another expression
    // we've probably already checked.
    value->validate();
}

const TypeExpr *ExprID::type() const {
    return type_of;
}

void ExprID::rvalue(std::ostream &) const {
}

void ExprID::generate(std::ostream &out) const {
    out << "TODO " << id;
}

void ExprID::lvalue(std::ostream &out) const {
    auto l = dynamic_cast<const Lvalue*>(value);
    assert(l != nullptr);
    l->lvalue(out);
}

ExprID::~ExprID() {
    delete value;
}

Var::Var(const VarDecl *decl, const location &loc, Indexer&)
  : Lvalue(loc), decl(decl->clone()) {
}

Var::Var(const Var &other):
  Lvalue(other), decl(other.decl->clone()) {
}

void swap(Var &x, Var &y) noexcept {
    using std::swap;
    swap(x.loc, y.loc);
    swap(x.decl, y.decl);
}

Var &Var::operator=(Var other) {
    swap(*this, other);
    return *this;
}

Var *Var::clone() const {
    return new Var(*this);
}

bool Var::constant() const {
    return false;
}

const TypeExpr *Var::type() const {
    return decl->type;
}

void Var::rvalue(std::ostream &out) const {
    if (decl->local) {
        out << "model_" << decl->name;
    } else {
        out << "state_read_" << decl->name << "(s)";
    }
}

void Var::lvalue(std::ostream &out) const {
    if (decl->local) {
        out << "model_" << decl->name;
    } else {
        out << "state_reference_" << decl->name << "(s)";
    }
}

Var::~Var() {
    delete decl;
}

void Var::generate(std::ostream &) const {
    // TODO
}

Field::Field(Lvalue *record, const std::string &field, const location &loc,
  Indexer&)
  : Lvalue(loc), record(record), field(field) {
}

Field::Field(const Field &other):
  Lvalue(other), record(other.record->clone()), field(other.field) {
}

void swap(Field &x, Field &y) noexcept {
    using std::swap;
    swap(x.loc, y.loc);
    swap(x.record, y.record);
    swap(x.field, y.field);
}

Field &Field::operator=(Field other) {
    swap(*this, other);
    return *this;
}

Field *Field::clone() const {
    return new Field(*this);
}

bool Field::constant() const {
    return record->constant();
}

void Field::rvalue(std::ostream &out) const {
    const TypeExpr *t = record->type();
    assert(t != nullptr && "root of field reference with no type");
    auto r = dynamic_cast<const Record*>(t);
    assert(r != nullptr && "root of field reference with non-record type");

    const SimpleTypeExpr *f = nullptr;
    for (const VarDecl *v : r->fields) {
        if (v->name == field) {
            f = dynamic_cast<const SimpleTypeExpr*>(v->type);
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

const TypeExpr *Field::type() const {
    // TODO
    return nullptr;
}

void Field::lvalue(std::ostream &out) const {
    const TypeExpr *t = record->type();
    assert(t != nullptr && "root of field reference with no type");
    auto r = dynamic_cast<const Record*>(t);
    assert(r != nullptr && "root of field reference with non-record type");
    r->field_referencer(out, field);
    out << "(";
    record->lvalue(out);
    out << ")";
}

void Field::generate(std::ostream &out) const {
    out << "(" << *record << "." << field << ")";
}

Field::~Field() {
    delete record;
}

Element::Element(Lvalue *array, Expr *index, const location &loc, Indexer&)
  : Lvalue(loc), array(array), index(index) {
}

Element::Element(const Element &other):
    Lvalue(other), array(other.array->clone()), index(other.index->clone()) {
}

void swap(Element &x, Element &y) noexcept {
    using std::swap;
    swap(x.loc, y.loc);
    swap(x.array, y.array);
    swap(x.index, y.index);
}

Element &Element::operator=(Element other) {
    swap(*this, other);
    return *this;
}

Element *Element::clone() const {
    return new Element(*this);
}

Element::~Element() {
    delete array;
    delete index;
}

bool Element::constant() const {
    return array->constant() && index->constant();
}

const TypeExpr *Element::type() const {
    // TODO
    return nullptr;
}

void Element::rvalue(std::ostream &out) const {
    const TypeExpr *t = array->type();
    assert(t != nullptr && "root of element reference with no type");
    auto a = dynamic_cast<const Array*>(t);
    assert(a != nullptr && "root of element reference with non-array type");
    auto e = dynamic_cast<const SimpleTypeExpr*>(a->element_type);

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

void Element::lvalue(std::ostream &out) const {
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

void Element::generate(std::ostream &out) const {
    out << "(" << *array << "[" << *index << "])";
}

Quantifier::Quantifier(const std::string &name, TypeExpr *type,
  const location &loc, Indexer &indexer)
  : Node(loc), var(new VarDecl(name, type, loc, indexer)) {
}

Quantifier::Quantifier(const std::string &name, Expr *from, Expr *to,
  const location &loc, Indexer& indexer)
  : Quantifier(loc, name, from, to, nullptr, indexer) {
}

Quantifier::Quantifier(const std::string &name, Expr *from, Expr *to, Expr *step,
  const location &loc, Indexer &indexer)
  : Quantifier(loc, name, from, to, step, indexer) {
}

Quantifier::Quantifier(const location &loc, const std::string &name, Expr *from,
  Expr *to, Expr *step, Indexer &indexer)
  : Node(loc),
    var(new VarDecl(name, new Range(from, to, loc, indexer), loc, indexer)),
    step(step) {
}

Quantifier::Quantifier(const Quantifier &other):
  Node(other), var(other.var->clone()),
  step(other.step == nullptr ? nullptr : other.step->clone()) {
}

Quantifier &Quantifier::operator=(Quantifier other) {
    swap(*this, other);
    return *this;
}

void swap(Quantifier &x, Quantifier &y) noexcept {
    using std::swap;
    swap(x.loc, y.loc);
    swap(x.var, y.var);
    swap(x.step, y.step);
}

Quantifier *Quantifier::clone() const {
    return new Quantifier(*this);
}

Quantifier::~Quantifier() {
    delete var;
    delete step;
}

void Quantifier::generate(std::ostream &out) const {
    // TODO: needs some more work
    out << "for(" << var->name << "...";
}

Exists::Exists(Quantifier *quantifier, Expr *expr, const location &loc, Indexer&)
  : Expr(loc), quantifier(quantifier), expr(expr) {
}

Exists::Exists(const Exists &other):
  Expr(other), quantifier(other.quantifier->clone()), expr(other.expr->clone()) {
}

Exists &Exists::operator=(Exists other) {
    swap(*this, other);
    return *this;
}

void swap(Exists &x, Exists &y) noexcept {
    using std::swap;
    swap(x.loc, y.loc);
    swap(x.quantifier, y.quantifier);
    swap(x.expr, y.expr);
}

Exists *Exists::clone() const {
    return new Exists(*this);
}

bool Exists::constant() const {
    return expr->constant();
}

const TypeExpr *Exists::type() const {
    return &Boolean;
}

void Exists::rvalue(std::ostream &out) const {
    out << "({bool r=false;for(int64_t model_" << quantifier->var->name << "=";
    quantifier->var->type->generate_min(out);
    out << ";;model_" << quantifier->var->name << "=add(model_"
      << quantifier->var->name << ",";
    if (quantifier->step != nullptr) {
        quantifier->step->rvalue(out);
    } else {
        out << "1";
    }
    out << ")){r|=";
    expr->rvalue(out);
    out << ";if(r||" << quantifier->var->name << "==";
    quantifier->var->type->generate_max(out);
    out << "){break;}}r;})";
}

Exists::~Exists() {
    delete quantifier;
    delete expr;
}

void Exists::generate(std::ostream &out) const {
    out << "({bool ru_g_TODO=false;" << *quantifier << "{if(" << *expr
      << "){ru_g_TODO=true;break;}}ru_g_TODO;})";
}

Forall::Forall(Quantifier *quantifier, Expr *expr, const location &loc, Indexer&)
  : Expr(loc), quantifier(quantifier), expr(expr) {
}

Forall::Forall(const Forall &other):
  Expr(other), quantifier(other.quantifier->clone()), expr(other.expr->clone()) {
}

Forall &Forall::operator=(Forall other) {
    swap(*this, other);
    return *this;
}

void swap(Forall &x, Forall &y) noexcept {
    using std::swap;
    swap(x.loc, y.loc);
    swap(x.quantifier, y.quantifier);
    swap(x.expr, y.expr);
}

Forall *Forall::clone() const {
    return new Forall(*this);
}

bool Forall::constant() const {
    return expr->constant();
}

const TypeExpr *Forall::type() const {
    return &Boolean;
}

void Forall::rvalue(std::ostream &out) const {
    out << "({bool r=true;for(int64_t model_" << quantifier->var->name << "=";
    quantifier->var->type->generate_min(out);
    out << ";;model_" << quantifier->var->name << "=add(model_"
      << quantifier->var->name << ",";
    if (quantifier->step != nullptr) {
        quantifier->step->rvalue(out);
    } else {
        out << "1";
    }
    out << ")){r&=";
    expr->rvalue(out);
    out << ";if(!r||" << quantifier->var->name << "==";
    quantifier->var->type->generate_max(out);
    out << "){break;}}r;})";
}

Forall::~Forall() {
    delete quantifier;
    delete expr;
}

void Forall::generate(std::ostream &out) const {
    out << "({bool ru_g_TODO=true;" << *quantifier << "{if(" << *expr
      << "){ru_g_TODO=false;break;}}ru_g_TODO;})";
}

}
