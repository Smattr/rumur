#include <cassert>
#include <cstddef>
#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/Function.h>
#include <rumur/Model.h>
#include <rumur/Node.h>
#include <rumur/Number.h>
#include <rumur/Property.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>
#include <rumur/TypeExpr.h>
#include <rumur/traverse.h>
#include <vector>

namespace rumur {

Node::Node(const location &loc_):
  loc(loc_) {
}

template<typename T>
static T pop(std::vector<T> &vs) {
  assert(!vs.empty() && "popping an empty vector");
  T t = vs[0];
  vs.erase(vs.begin());
  return t;
}

static void expand(Node &n, std::vector<Node*> &to) {

  class Expander : public BaseTraversal<> {

   private:
    std::vector<Node*> &target;

   public:
    explicit Expander(std::vector<Node*> &target_): target(target_) { }

    // Note that we do not actually do any descending into child nodes in this
    // “traversal.” This is less of a traversal and more of a single dispatch.

    void visit_add(Add &n) final {
      visit_bexpr(n);
    }

    void visit_aliasdecl(AliasDecl &n) final {
      target.insert(target.begin(), n.value.get());
    }

    void visit_aliasrule(AliasRule &n) final {
      auto it = target.begin();
      for (Ptr<AliasDecl> &a : n.aliases) {
        it = target.insert(it, a.get());
        ++it;
      }
      for (Ptr<Rule> &r : n.rules) {
        it = target.insert(it, r.get());
        ++it;
      }
    }

    void visit_aliasstmt(AliasStmt &n) final {
      auto it = target.begin();
      for (Ptr<AliasDecl> &a : n.aliases) {
        it = target.insert(it, a.get());
        ++it;
      }
      for (Ptr<Stmt> &s : n.body) {
        it = target.insert(it, s.get());
        ++it;
      }
    }

    void visit_and(And &n) final {
      visit_bexpr(n);
    }

    void visit_array(Array &n) final {
      target.insert(target.begin(),
        { n.index_type.get(), n.element_type.get() });
    }

    void visit_assignment(Assignment &n) final {
      target.insert(target.begin(), { n.lhs.get(), n.rhs.get() });
    }

    void visit_clear(Clear &n) final {
      target.insert(target.begin(), n.rhs.get());
    }

    void visit_constdecl(ConstDecl &n) final {
      target.insert(target.begin(), n.value.get());
    }

    void visit_div(Div &n) final {
      visit_bexpr(n);
    }

    void visit_element(Element &n) final {
      target.insert(target.begin(), { n.array.get(), n.index.get() });
    }

    void visit_enum(Enum&) final { }

    void visit_eq(Eq &n) final {
      visit_bexpr(n);
    }

    void visit_errorstmt(ErrorStmt&) final { }

    void visit_exists(Exists &n) final {
      target.insert(target.begin(), { &n.quantifier, n.expr.get() });
    }

    void visit_exprid(ExprID&) final { }

    void visit_field(Field &n) final {
      target.insert(target.begin(), n.record.get());
    }

    void visit_for(For &n) final {
      auto it = target.begin();
      it = target.insert(it, &n.quantifier);
      ++it;
      for (Ptr<Stmt> &s : n.body) {
        it = target.insert(it, s.get());
        ++it;
      }
    }

    void visit_forall(Forall &n) final {
      target.insert(target.begin(), { &n.quantifier, n.expr.get() });
    }

    void visit_function(Function &n) final {
      auto it = target.begin();
      for (Ptr<VarDecl> &p : n.parameters) {
        it = target.insert(it, p.get());
        ++it;
      }
      if (n.return_type != nullptr) {
        it = target.insert(it, n.return_type.get());
        ++it;
      }
      for (Ptr<Decl> &d : n.decls) {
        it = target.insert(it, d.get());
        ++it;
      }
      for (Ptr<Stmt> &s : n.body) {
        it = target.insert(it, s.get());
        ++it;
      }
    }

    void visit_functioncall(FunctionCall &n) final {
      auto it = target.begin();
      for (Ptr<Expr> &a : n.arguments) {
        it = target.insert(it, a.get());
        ++it;
      }
    }

    void visit_geq(Geq &n) final {
      visit_bexpr(n);
    }

    void visit_gt(Gt &n) final {
      visit_bexpr(n);
    }

    void visit_if(If &n) final {
      auto it = target.begin();
      for (IfClause &c : n.clauses) {
        it = target.insert(it, &c);
        ++it;
      }
    }

    void visit_ifclause(IfClause &n) final {
      auto it = target.begin();
      if (n.condition != nullptr) {
        it = target.insert(it, n.condition.get());
        ++it;
      }
      for (Ptr<Stmt> &s : n.body) {
        it = target.insert(it, s.get());
        ++it;
      }
    }

    void visit_implication(Implication &n) final {
      visit_bexpr(n);
    }

    void visit_isundefined(IsUndefined &n) final {
      target.insert(target.begin(), n.expr.get());
    }

    void visit_leq(Leq &n) final {
      visit_bexpr(n);
    }

    void visit_lt(Lt &n) final {
      visit_bexpr(n);
    }

    void visit_mod(Mod &n) final {
      visit_bexpr(n);
    }

    void visit_model(Model &n) final {
      auto it = target.begin();
      for (Ptr<Decl> &d : n.decls) {
        it = target.insert(it, d.get());
        ++it;
      }
      for (Ptr<Function> &f : n.functions) {
        it = target.insert(it, f.get());
        ++it;
      }
      for (Ptr<Rule> &r : n.rules) {
        it = target.insert(it, r.get());
        ++it;
      }
    }

    void visit_mul(Mul &n) final {
      visit_bexpr(n);
    }

    void visit_negative(Negative &n) final {
      visit_uexpr(n);
    }

    void visit_neq(Neq &n) final {
      visit_bexpr(n);
    }

    void visit_not(Not &n) final {
      visit_uexpr(n);
    }

    void visit_number(Number&) final { }

    void visit_or(Or &n) final {
      visit_bexpr(n);
    }

    void visit_procedurecall(ProcedureCall &n) final {
      target.insert(target.begin(), &n.call);
    }

    void visit_property(Property &n) final {
      target.insert(target.begin(), n.expr.get());
    }

    void visit_propertyrule(PropertyRule &n) final {
      auto it = target.begin();
      for (Quantifier &q : n.quantifiers) {
        it = target.insert(it, &q);
        ++it;
      }
      target.insert(it, &n.property);
    }

    void visit_propertystmt(PropertyStmt &n) final {
      target.insert(target.begin(), &n.property);
    }

    void visit_put(Put &n) final {
      if (n.expr != nullptr) {
        target.insert(target.begin(), n.expr.get());
      }
    }

    void visit_quantifier(Quantifier &n) final {
      auto it = target.begin();
      if (n.type != nullptr) {
        it = target.insert(it, n.type.get());
        ++it;
      }
      if (n.from != nullptr) {
        it = target.insert(it, n.from.get());
        ++it;
      }
      if (n.to != nullptr) {
        it = target.insert(it, n.to.get());
        ++it;
      }
      if (n.step != nullptr) {
        it = target.insert(it, n.step.get());
        ++it;
      }
    }

    void visit_range(Range &n) final {
      target.insert(target.begin(), { n.min.get(), n.max.get() });
    }

    void visit_record(Record &n) final {
      auto it = target.begin();
      for (Ptr<VarDecl> &f : n.fields) {
        it = target.insert(it, f.get());
        ++it;
      }
    }

    void visit_return(Return &n) final {
      if (n.expr != nullptr) {
        target.insert(target.begin(), n.expr.get());
      }
    }

    void visit_ruleset(Ruleset &n) final {
      auto it = target.begin();
      for (Quantifier &q : n.quantifiers) {
        it = target.insert(it, &q);
        ++it;
      }
      for (Ptr<Rule> &r : n.rules) {
        it = target.insert(it, r.get());
        ++it;
      }
    }

    void visit_scalarset(Scalarset &n) final {
      target.insert(target.begin(), n.bound.get());
    }

    void visit_simplerule(SimpleRule &n) final {
      auto it = target.begin();
      for (Quantifier &q : n.quantifiers) {
        it = target.insert(it, &q);
        ++it;
      }
      if (n.guard != nullptr) {
        it = target.insert(it, n.guard.get());
        ++it;
      }
      for (Ptr<Decl> &d : n.decls) {
        it = target.insert(it, d.get());
        ++it;
      }
      for (Ptr<Stmt> &s : n.body) {
        it = target.insert(it, s.get());
        ++it;
      }
    }

    void visit_startstate(StartState &n) final {
      auto it = target.begin();
      for (Quantifier &q : n.quantifiers) {
        it = target.insert(it, &q);
        ++it;
      }
      for (Ptr<Decl> &d : n.decls) {
        it = target.insert(it, d.get());
        ++it;
      }
      for (Ptr<Stmt> &s : n.body) {
        it = target.insert(it, s.get());
        ++it;
      }
    }

    void visit_sub(Sub &n) final {
      visit_bexpr(n);
    }

    void visit_switch(Switch &n) final {
      auto it = target.begin();
      it = target.insert(it, n.expr.get());
      ++it;
      for (SwitchCase &c : n.cases) {
        it = target.insert(it, &c);
        ++it;
      }
    }

    void visit_switchcase(SwitchCase &n) final {
      auto it = target.begin();
      for (Ptr<Expr> &m : n.matches) {
        it = target.insert(it, m.get());
        ++it;
      }
      for (Ptr<Stmt> &s : n.body) {
        it = target.insert(it, s.get());
        ++it;
      }
    }

    void visit_ternary(Ternary &n) final {
      target.insert(target.begin(), { n.cond.get(), n.lhs.get(), n.rhs.get() });
    }

    void visit_typedecl(TypeDecl &n) final {
      target.insert(target.begin(), n.value.get());
    }

    void visit_typeexprid(TypeExprID&) final { }

    void visit_undefine(Undefine &n) final {
      target.insert(target.begin(), n.rhs.get());
    }

    void visit_vardecl(VarDecl &n) final {
      if (n.type != nullptr) {
        target.insert(target.begin(), n.type.get());
      }
    }

    void visit_while(While &n) final {
      auto it = target.begin();
      it = target.insert(it, n.condition.get());
      ++it;
      for (Ptr<Stmt> &s : n.body) {
        it = target.insert(it, s.get());
        ++it;
      }
    }

    virtual ~Expander() = default;

   private:
    void visit_bexpr(BinaryExpr &n) {
      target.insert(target.begin(), { n.lhs.get(), n.rhs.get() });
    }

    void visit_uexpr(UnaryExpr &n) {
      target.insert(target.begin(), n.rhs.get());
    }
  };

  Expander e(to);
  e.dispatch(n);
}

Node::PreorderIterator::PreorderIterator() { }

Node::PreorderIterator::PreorderIterator(Node &base) {
  expand(base, remaining);
}

Node::PreorderIterator &Node::PreorderIterator::operator++() {

  assert(!remaining.empty() && "advancing an empty iterator");

  // the implementation here does a pre-order traversal, but the API is
  // documented in Node.h to traverse in an unspecified order to leave us the
  // freedom to change this in future

  // remove the current node we are at
  Node *next = pop(remaining);

  // add its children to the pending list
  expand(*next, remaining);

  return *this;
}

Node::PreorderIterator Node::PreorderIterator::operator++(int) {
  Iterator it = *this;
  ++*this;
  return it;
}

Node *Node::PreorderIterator::operator*() {

  assert(!remaining.empty() && "dereferencing empty iterator");
  return remaining[0];
}

bool Node::PreorderIterator::operator==(const PreorderIterator &other) const {
  return remaining == other.remaining;
}

bool Node::PreorderIterator::operator!=(const PreorderIterator &other) const {
  return !(*this == other);
}

Node::Iterator Node::begin() {
  Iterator it(*this);
  return it;
}

Node::Iterator Node::end() {
  Iterator it;
  return it;
}

// replicate the above for a const iterator

static void expand(const Node &n, std::vector<const Node*> &to) {

  class Expander : public ConstBaseTraversal<> {

   private:
    std::vector<const Node*> &target;

   public:
    explicit Expander(std::vector<const Node*> &target_): target(target_) { }

    void visit_add(const Add &n) final {
      visit_bexpr(n);
    }

    void visit_aliasdecl(const AliasDecl &n) final {
      target.insert(target.begin(), n.value.get());
    }

    void visit_aliasrule(const AliasRule &n) final {
      auto it = target.begin();
      for (const Ptr<AliasDecl> &a : n.aliases) {
        it = target.insert(it, a.get());
        ++it;
      }
      for (const Ptr<Rule> &r : n.rules) {
        it = target.insert(it, r.get());
        ++it;
      }
    }

    void visit_aliasstmt(const AliasStmt &n) final {
      auto it = target.begin();
      for (const Ptr<AliasDecl> &a : n.aliases) {
        it = target.insert(it, a.get());
        ++it;
      }
      for (const Ptr<Stmt> &s : n.body) {
        it = target.insert(it, s.get());
        ++it;
      }
    }

    void visit_and(const And &n) final {
      visit_bexpr(n);
    }

    void visit_array(const Array &n) final {
      target.insert(target.begin(),
        { n.index_type.get(), n.element_type.get() });
    }

    void visit_assignment(const Assignment &n) final {
      target.insert(target.begin(), { n.lhs.get(), n.rhs.get() });
    }

    void visit_clear(const Clear &n) final {
      target.insert(target.begin(), n.rhs.get());
    }

    void visit_constdecl(const ConstDecl &n) final {
      target.insert(target.begin(), n.value.get());
    }

    void visit_div(const Div &n) final {
      visit_bexpr(n);
    }

    void visit_element(const Element &n) final {
      target.insert(target.begin(), { n.array.get(), n.index.get() });
    }

    void visit_enum(const Enum&) final { }

    void visit_eq(const Eq &n) final {
      visit_bexpr(n);
    }

    void visit_errorstmt(const ErrorStmt&) final { }

    void visit_exists(const Exists &n) final {
      target.insert(target.begin(), { &n.quantifier, n.expr.get() });
    }

    void visit_exprid(const ExprID&) final { }

    void visit_field(const Field &n) final {
      target.insert(target.begin(), n.record.get());
    }

    void visit_for(const For &n) final {
      auto it = target.begin();
      it = target.insert(it, &n.quantifier);
      ++it;
      for (const Ptr<Stmt> &s : n.body) {
        it = target.insert(it, s.get());
        ++it;
      }
    }

    void visit_forall(const Forall &n) final {
      target.insert(target.begin(), { &n.quantifier, n.expr.get() });
    }

    void visit_function(const Function &n) final {
      auto it = target.begin();
      for (const Ptr<VarDecl> &p : n.parameters) {
        it = target.insert(it, p.get());
        ++it;
      }
      if (n.return_type != nullptr) {
        it = target.insert(it, n.return_type.get());
        ++it;
      }
      for (const Ptr<Decl> &d : n.decls) {
        it = target.insert(it, d.get());
        ++it;
      }
      for (const Ptr<Stmt> &s : n.body) {
        it = target.insert(it, s.get());
        ++it;
      }
    }

    void visit_functioncall(const FunctionCall &n) final {
      auto it = target.begin();
      for (const Ptr<Expr> &a : n.arguments) {
        it = target.insert(it, a.get());
        ++it;
      }
    }

    void visit_geq(const Geq &n) final {
      visit_bexpr(n);
    }

    void visit_gt(const Gt &n) final {
      visit_bexpr(n);
    }

    void visit_if(const If &n) final {
      auto it = target.begin();
      for (const IfClause &c : n.clauses) {
        it = target.insert(it, &c);
        ++it;
      }
    }

    void visit_ifclause(const IfClause &n) final {
      auto it = target.begin();
      if (n.condition != nullptr) {
        it = target.insert(it, n.condition.get());
        ++it;
      }
      for (const Ptr<Stmt> &s : n.body) {
        it = target.insert(it, s.get());
        ++it;
      }
    }

    void visit_implication(const Implication &n) final {
      visit_bexpr(n);
    }

    void visit_isundefined(const IsUndefined &n) final {
      target.insert(target.begin(), n.expr.get());
    }

    void visit_leq(const Leq &n) final {
      visit_bexpr(n);
    }

    void visit_lt(const Lt &n) final {
      visit_bexpr(n);
    }

    void visit_mod(const Mod &n) final {
      visit_bexpr(n);
    }

    void visit_model(const Model &n) final {
      auto it = target.begin();
      for (const Ptr<Decl> &d : n.decls) {
        it = target.insert(it, d.get());
        ++it;
      }
      for (const Ptr<Function> &f : n.functions) {
        it = target.insert(it, f.get());
        ++it;
      }
      for (const Ptr<Rule> &r : n.rules) {
        it = target.insert(it, r.get());
        ++it;
      }
    }

    void visit_mul(const Mul &n) final {
      visit_bexpr(n);
    }

    void visit_negative(const Negative &n) final {
      visit_uexpr(n);
    }

    void visit_neq(const Neq &n) final {
      visit_bexpr(n);
    }

    void visit_not(const Not &n) final {
      visit_uexpr(n);
    }

    void visit_number(const Number&) final { }

    void visit_or(const Or &n) final {
      visit_bexpr(n);
    }

    void visit_procedurecall(const ProcedureCall &n) final {
      target.insert(target.begin(), &n.call);
    }

    void visit_property(const Property &n) final {
      target.insert(target.begin(), n.expr.get());
    }

    void visit_propertyrule(const PropertyRule &n) final {
      auto it = target.begin();
      for (const Quantifier &q : n.quantifiers) {
        it = target.insert(it, &q);
        ++it;
      }
      target.insert(it, &n.property);
    }

    void visit_propertystmt(const PropertyStmt &n) final {
      target.insert(target.begin(), &n.property);
    }

    void visit_put(const Put &n) final {
      if (n.expr != nullptr) {
        target.insert(target.begin(), n.expr.get());
      }
    }

    void visit_quantifier(const Quantifier &n) final {
      auto it = target.begin();
      if (n.type != nullptr) {
        it = target.insert(it, n.type.get());
        ++it;
      }
      if (n.from != nullptr) {
        it = target.insert(it, n.from.get());
        ++it;
      }
      if (n.to != nullptr) {
        it = target.insert(it, n.to.get());
        ++it;
      }
      if (n.step != nullptr) {
        it = target.insert(it, n.step.get());
        ++it;
      }
    }

    void visit_range(const Range &n) final {
      target.insert(target.begin(), { n.min.get(), n.max.get() });
    }

    void visit_record(const Record &n) final {
      auto it = target.begin();
      for (const Ptr<VarDecl> &f : n.fields) {
        it = target.insert(it, f.get());
        ++it;
      }
    }

    void visit_return(const Return &n) final {
      if (n.expr != nullptr) {
        target.insert(target.begin(), n.expr.get());
      }
    }

    void visit_ruleset(const Ruleset &n) final {
      auto it = target.begin();
      for (const Quantifier &q : n.quantifiers) {
        it = target.insert(it, &q);
        ++it;
      }
      for (const Ptr<Rule> &r : n.rules) {
        it = target.insert(it, r.get());
        ++it;
      }
    }

    void visit_scalarset(const Scalarset &n) final {
      target.insert(target.begin(), n.bound.get());
    }

    void visit_simplerule(const SimpleRule &n) final {
      auto it = target.begin();
      for (const Quantifier &q : n.quantifiers) {
        it = target.insert(it, &q);
        ++it;
      }
      if (n.guard != nullptr) {
        it = target.insert(it, n.guard.get());
        ++it;
      }
      for (const Ptr<Decl> &d : n.decls) {
        it = target.insert(it, d.get());
        ++it;
      }
      for (const Ptr<Stmt> &s : n.body) {
        it = target.insert(it, s.get());
        ++it;
      }
    }

    void visit_startstate(const StartState &n) final {
      auto it = target.begin();
      for (const Quantifier &q : n.quantifiers) {
        it = target.insert(it, &q);
        ++it;
      }
      for (const Ptr<Decl> &d : n.decls) {
        it = target.insert(it, d.get());
        ++it;
      }
      for (const Ptr<Stmt> &s : n.body) {
        it = target.insert(it, s.get());
        ++it;
      }
    }

    void visit_sub(const Sub &n) final {
      visit_bexpr(n);
    }

    void visit_switch(const Switch &n) final {
      auto it = target.begin();
      it = target.insert(it, n.expr.get());
      ++it;
      for (const SwitchCase &c : n.cases) {
        it = target.insert(it, &c);
        ++it;
      }
    }

    void visit_switchcase(const SwitchCase &n) final {
      auto it = target.begin();
      for (const Ptr<Expr> &m : n.matches) {
        it = target.insert(it, m.get());
        ++it;
      }
      for (const Ptr<Stmt> &s : n.body) {
        it = target.insert(it, s.get());
        ++it;
      }
    }

    void visit_ternary(const Ternary &n) final {
      target.insert(target.begin(), { n.cond.get(), n.lhs.get(), n.rhs.get() });
    }

    void visit_typedecl(const TypeDecl &n) final {
      target.insert(target.begin(), n.value.get());
    }

    void visit_typeexprid(const TypeExprID&) final { }

    void visit_undefine(const Undefine &n) final {
      target.insert(target.begin(), n.rhs.get());
    }

    void visit_vardecl(const VarDecl &n) final {
      if (n.type != nullptr) {
        target.insert(target.begin(), n.type.get());
      }
    }

    void visit_while(const While &n) final {
      auto it = target.begin();
      it = target.insert(it, n.condition.get());
      ++it;
      for (const Ptr<Stmt> &s : n.body) {
        it = target.insert(it, s.get());
        ++it;
      }
    }

    virtual ~Expander() = default;

   private:
    void visit_bexpr(const BinaryExpr &n) {
      target.insert(target.begin(), { n.lhs.get(), n.rhs.get() });
    }

    void visit_uexpr(const UnaryExpr &n) {
      target.insert(target.begin(), n.rhs.get());
    }
  };

  Expander e(to);
  e.dispatch(n);
}

Node::ConstPreorderIterator::ConstPreorderIterator() { }

Node::ConstPreorderIterator::ConstPreorderIterator(const Node &base) {
  expand(base, remaining);
}

Node::ConstPreorderIterator &Node::ConstPreorderIterator::operator++() {

  assert(!remaining.empty() && "advancing an empty iterator");

  // the implementation here does a pre-order traversal, but the API is
  // documented in Node.h to traverse in an unspecified order to leave us the
  // freedom to change this in future

  // remove the current node we are at
  const Node *next = pop(remaining);

  // add its children to the pending list
  expand(*next, remaining);

  return *this;
}

Node::ConstPreorderIterator Node::ConstPreorderIterator::operator++(int) {
  ConstPreorderIterator it = *this;
  ++*this;
  return it;
}

const Node *Node::ConstPreorderIterator::operator*() {

  assert(!remaining.empty() && "dereferencing empty iterator");
  return remaining[0];
}

bool Node::ConstPreorderIterator::operator==(const ConstPreorderIterator &other) const {
  return remaining == other.remaining;
}

bool Node::ConstPreorderIterator::operator!=(const ConstPreorderIterator &other) const {
  return !(*this == other);
}

Node::ConstIterator Node::begin() const {
  ConstIterator it(*this);
  return it;
}

Node::ConstIterator Node::end() const {
  ConstIterator it;
  return it;
}

Node::PreorderWrapper::PreorderWrapper(Node &root_): root(root_) { }

Node::PreorderIterator Node::PreorderWrapper::begin() {
  return PreorderIterator(root);
}

Node::PreorderIterator Node::PreorderWrapper::end() {
  return PreorderIterator();
}

Node::ConstPreorderWrapper::ConstPreorderWrapper(const Node &root_): root(root_)
  { }

Node::ConstPreorderIterator Node::ConstPreorderWrapper::begin() {
  return ConstPreorderIterator(root);
}

Node::ConstPreorderIterator Node::ConstPreorderWrapper::end() {
  return ConstPreorderIterator();
}

Node::PostorderIterator::PostorderIterator() { }

Node::PostorderIterator::PostorderIterator(Node &root) {
  remaining.push_back(std::make_pair(&root, false));
  expand_head();
}

Node::PostorderIterator &Node::PostorderIterator::operator++() {

  assert(!remaining.empty() && "incrementing an empty iterator");

  // discard the current node
  assert(remaining[0].second && "current node has not been expanded");
  remaining.erase(remaining.begin());

  // expand the next pending until we have an expanded node at the head of our
  // queue
  expand_head();

  return *this;
}

void Node::PostorderIterator::expand_head() {

  while (!remaining.empty() && !remaining[0].second) {

    // take the current node and expand it
    Node *current = remaining[0].first;
    std::vector<Node*> vs;
    expand(*current, vs);
    remaining[0].second = true;

    // we insert new nodes as unexpanded first
    auto it = remaining.begin();
    for (Node *n : vs) {
      it = remaining.insert(it, std::make_pair(n, false));
      it++;
    }
  }
}

Node::PostorderIterator Node::PostorderIterator::operator++(int) {
  PostorderIterator it = *this;
  ++*this;
  return it;
}

Node *Node::PostorderIterator::operator*() {
  assert(!remaining.empty() && "dereferencing an empty iterator");
  return remaining[0].first;
}

bool Node::PostorderIterator::operator==(const PostorderIterator &other) const {
  return remaining == other.remaining;
}

bool Node::PostorderIterator::operator!=(const PostorderIterator &other) const {
  return !(*this == other);
}

// duplicate the above for a const post-order

Node::ConstPostorderIterator::ConstPostorderIterator() { }

Node::ConstPostorderIterator::ConstPostorderIterator(const Node &root) {
  remaining.push_back(std::make_pair(&root, false));
  expand_head();
}

Node::ConstPostorderIterator &Node::ConstPostorderIterator::operator++() {

  assert(!remaining.empty() && "incrementing an empty iterator");

  // discard the current node
  assert(remaining[0].second && "current node has not been expanded");
  remaining.erase(remaining.begin());

  // expand the next pending until we have an expanded node at the head of our
  // queue
  expand_head();

  return *this;
}

void Node::ConstPostorderIterator::expand_head() {

  while (!remaining.empty() && !remaining[0].second) {

    // take the current node and expand it
    const Node *current = remaining[0].first;
    std::vector<const Node*> vs;
    expand(*current, vs);
    remaining[0].second = true;

    // we insert new nodes as unexpanded first
    auto it = remaining.begin();
    for (const Node *n : vs) {
      it = remaining.insert(it, std::make_pair(n, false));
      it++;
    }
  }
}

Node::ConstPostorderIterator Node::ConstPostorderIterator::operator++(int) {
  ConstPostorderIterator it = *this;
  ++*this;
  return it;
}

const Node *Node::ConstPostorderIterator::operator*() {
  assert(!remaining.empty() && "dereferencing an empty iterator");
  return remaining[0].first;
}

bool Node::ConstPostorderIterator::operator==(const ConstPostorderIterator &other) const {
  return remaining == other.remaining;
}

bool Node::ConstPostorderIterator::operator!=(const ConstPostorderIterator &other) const {
  return !(*this == other);
}

}
