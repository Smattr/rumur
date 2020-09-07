#pragma once

#include <cstddef>
#include "location.hh"
#include <memory>
#include <rumur/Decl.h>
#include <rumur/Node.h>
#include <rumur/Ptr.h>
#include <rumur/Stmt.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <vector>

namespace rumur {

struct Function : public Node {

  std::string name;
  std::vector<Ptr<VarDecl>> parameters;
  Ptr<TypeExpr> return_type;
  std::vector<Ptr<Decl>> decls;
  std::vector<Ptr<Stmt>> body;

  Function(const std::string &name_,
    const std::vector<Ptr<VarDecl>> &parameters_,
    const Ptr<TypeExpr> &return_type_,
    const std::vector<Ptr<Decl>> &decls_,
    const std::vector<Ptr<Stmt>> &body_, const location &loc_);
  virtual ~Function() = default;
  Function *clone() const final;
  void validate() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  // is this function side effect free?
  bool is_pure() const;

  // does this function contain calls to itself?
  bool is_recursive() const;
};

}
