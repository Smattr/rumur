#pragma once

#include "location.hh"
#include <cstddef>
#include <memory>
#include <rumur/Decl.h>
#include <rumur/Node.h>
#include <rumur/Ptr.h>
#include <rumur/Stmt.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <vector>

#ifndef RUMUR_API_WITH_RTTI
#define RUMUR_API_WITH_RTTI __attribute__((visibility("default")))
#endif

namespace rumur {

struct RUMUR_API_WITH_RTTI Function : public Node {

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
  Function *clone() const override;
  void validate() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  // is this function side effect free?
  bool is_pure() const;

  // does this function contain calls to itself?
  bool is_recursive() const;
};

} // namespace rumur
