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
  std::vector<std::shared_ptr<VarDecl>> parameters;
  std::shared_ptr<TypeExpr> return_type;
  std::vector<std::shared_ptr<Decl>> decls;
  std::vector<Ptr<Stmt>> body;

  Function() = delete;
  Function(const std::string &name_,
    std::vector<std::shared_ptr<VarDecl>> &&parameters_,
    std::shared_ptr<TypeExpr> return_type_,
    std::vector<std::shared_ptr<Decl>> &&decls_,
    const std::vector<Ptr<Stmt>> &body_, const location &loc_);
  Function(const Function &other);
  Function &operator=(Function other);
  friend void swap(Function &x, Function &y) noexcept;
  virtual ~Function() { }
  Function *clone() const final;
  bool operator==(const Node &other) const final;
  void validate() const final;
};

}
