#include <cstddef>
#include <memory>
#include <rumur/Decl.h>
#include <rumur/Function.h>
#include <rumur/Ptr.h>
#include <rumur/Stmt.h>
#include <rumur/TypeExpr.h>
#include <rumur/except.h>
#include <rumur/traverse.h>
#include <string>
#include <utility>
#include <vector>

namespace rumur {

Function::Function(const std::string &name_,
                   const std::vector<Ptr<VarDecl>> &parameters_,
                   const Ptr<TypeExpr> &return_type_,
                   const std::vector<Ptr<Decl>> &decls_,
                   const std::vector<Ptr<Stmt>> &body_, const location &loc_)
    : Node(loc_), name(name_), parameters(parameters_),
      return_type(return_type_), decls(decls_), body(body_) {}

Function *Function::clone() const { return new Function(*this); }

void Function::validate() const {

  /*Define a traversal that checks our contained return statements for
   * correctness.
   */
  class ReturnChecker : public ConstTraversal {

  private:
    const TypeExpr *return_type;

  public:
    ReturnChecker(const TypeExpr *rt) : return_type(rt) {}

    void visit_return(const Return &n) final {

      if (return_type == nullptr) {
        if (n.expr != nullptr)
          throw Error("statement returns a value from a procedure", n.loc);

      } else {

        if (n.expr == nullptr)
          throw Error("empty return statement in a function", n.loc);

        if (!n.expr->type()->coerces_to(*return_type))
          throw Error("returning incompatible typed value from a function",
                      n.loc);
      }
    }

    virtual ~ReturnChecker() = default;
  };

  // Run the checker
  ReturnChecker rt(return_type.get());
  for (auto &s : body)
    rt.dispatch(*s);
}

void Function::visit(BaseTraversal &visitor) { visitor.visit_function(*this); }

void Function::visit(ConstBaseTraversal &visitor) const {
  visitor.visit_function(*this);
}

bool Function::is_pure() const {

  // if the function takes a var parameter, consider it impure
  for (const Ptr<VarDecl> &p : parameters) {
    if (!p->is_readonly())
      return false;
  }

  // a traversal that looks for impure features
  class PurityDetector : public ConstTraversal {

  private:
    const Function *root;

    // is this a reference to a state variable?
    bool is_global_ref(const ExprDecl &expr) const {

      if (auto e = dynamic_cast<const AliasDecl *>(&expr))
        return is_global_ref(*e->value);

      if (auto e = dynamic_cast<const VarDecl *>(&expr))
        return e->is_in_state();

      return false;
    }

    bool is_global_ref(const Expr &expr) const {

      if (auto e = dynamic_cast<const ExprID *>(&expr))
        return is_global_ref(*e->value);

      if (auto e = dynamic_cast<const Field *>(&expr))
        return is_global_ref(*e->record);

      if (auto e = dynamic_cast<const Element *>(&expr))
        return is_global_ref(*e->array);

      return false;
    }

  public:
    bool pure = true;

    PurityDetector(const Function &root_) : root(&root_) {}

    void visit_assignment(const Assignment &n) final {
      pure &= !is_global_ref(*n.lhs);

      dispatch(*n.lhs);
      dispatch(*n.rhs);
    }

    void visit_clear(const Clear &n) final {
      pure &= !is_global_ref(*n.rhs);

      dispatch(*n.rhs);
    }

    void visit_errorstmt(const ErrorStmt &) final {
      // treat any error as a side effect
      pure = false;
    }

    void visit_functioncall(const FunctionCall &n) final {

      assert(n.function != nullptr && "unresolved function call");

      for (const Ptr<Expr> &a : n.arguments)
        dispatch(*a);

      // check if this is a recursive call, to avoid looping
      if (n.function->unique_id != root->unique_id)
        pure &= n.function->is_pure();
    }

    void visit_propertystmt(const PropertyStmt &) final {
      // treat any property statement as a side effect
      pure = false;
      // no need to further descend
    }

    void visit_put(const Put &) final {
      pure = false;
      // no need to descend into children
    }

    void visit_undefine(const Undefine &n) final {
      pure &= !is_global_ref(*n.rhs);

      dispatch(*n.rhs);
    }
  };

  // run the traversal on ourselves
  PurityDetector pd(*this);
  pd.dispatch(*this);

  return pd.pure;
}

bool Function::is_recursive() const {

  // a traversal that finds calls to a given function
  class CallFinder : public ConstTraversal {

  private:
    const Function *needle;

  public:
    bool found = false;

    explicit CallFinder(const Function &needle_) : needle(&needle_) {}

    void visit_functioncall(const FunctionCall &n) final {
      if (n.function != nullptr && n.function->unique_id == needle->unique_id)
        found = true;
    }

    virtual ~CallFinder() = default;
  };

  // run the traversal on ourselves
  CallFinder cf(*this);
  cf.dispatch(*this);

  return cf.found;
}

} // namespace rumur
