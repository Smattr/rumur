#include <cstddef>
#include <memory>
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Function.h>
#include <rumur/Ptr.h>
#include <rumur/Stmt.h>
#include <rumur/traverse.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>
#include "utils.h"
#include <vector>

namespace rumur {

Function::Function(const std::string &name_,
  std::vector<std::shared_ptr<VarDecl>> &&parameters_,
  std::shared_ptr<TypeExpr> return_type_,
  std::vector<std::shared_ptr<Decl>> &&decls_,
  const std::vector<Ptr<Stmt>> &body_, const location &loc_):
  Node(loc_), name(name_), parameters(parameters_), return_type(return_type_),
  decls(decls_), body(body_) { }

Function::Function(const Function &other):
  Node(other), name(other.name),
  return_type(other.return_type == nullptr ? nullptr : other.return_type->clone()),
  body(other.body) {

  for (const std::shared_ptr<VarDecl> &p : other.parameters)
    parameters.emplace_back(p->clone());

  for (const std::shared_ptr<Decl> &d : other.decls)
    decls.emplace_back(d->clone());
}

Function &Function::operator=(Function other) {
  swap(*this, other);
  return *this;
}

void swap(Function &x, Function &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.unique_id, y.unique_id);
  swap(x.name, y.name);
  swap(x.parameters, y.parameters);
  swap(x.return_type, y.return_type);
  swap(x.decls, y.decls);
  swap(x.body, y.body);
}

Function *Function::clone() const {
  return new Function(*this);
}

bool Function::operator==(const Node &other) const {
  auto o = dynamic_cast<const Function*>(&other);
  if (o == nullptr)
    return false;
  if (name != o->name)
    return false;
  if (!vector_eq(parameters, o->parameters))
    return false;
  if (return_type == nullptr) {
    if (o->return_type != nullptr)
      return false;
  } else {
    if (o->return_type == nullptr)
      return false;
    if (*return_type != *o->return_type)
      return false;
  }
  if (!vector_eq(decls, o->decls))
    return false;
  if (!vector_eq(body, o->body))
    return false;
  return true;
}

void Function::validate() const {

  /*Define a traversal that checks our contained return statements for
   * correctness.
   */
  class ReturnChecker : public ConstTraversal {

   private:
    const TypeExpr *return_type;

   public:
    ReturnChecker(const TypeExpr *rt): return_type(rt) { }

    void visit(const Return &n) final {

      if (return_type == nullptr) {
        if (n.expr != nullptr)
          throw Error("statement returns a value from a procedure", n.loc);

      } else {

        if (n.expr == nullptr)
          throw Error("empty return statement in a function", n.loc);

        if (n.expr->type() == nullptr) {
          if (!isa<Range>(return_type->resolve()))
            throw Error("returning a number from a function that does not "
              "return a range", n.loc);

        } else {
          if (*n.expr->type() != *return_type)
            throw Error("returning incompatible typed value from a function",
              n.loc);
        }
      }
    }

    virtual ~ReturnChecker() { }
  };

  // Run the checker
  ReturnChecker rt(return_type.get());
  for (auto &s : body)
    rt.dispatch(*s);
}

}
