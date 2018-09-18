#pragma once

#include "location.hh"
#include <memory>
#include <rumur/Decl.h>
#include <rumur/Node.h>
#include <rumur/Stmt.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <vector>

namespace rumur {

struct Parameter : public Node {

  std::shared_ptr<VarDecl> decl;
  bool by_reference;

  Parameter() = delete;
  Parameter(std::shared_ptr<VarDecl> decl_, bool by_reference_,
    const location &loc_);
  Parameter(const Parameter &other);
  Parameter &operator=(Parameter other);
  friend void swap(Parameter &x, Parameter &y) noexcept;
  virtual ~Parameter() { }
  Parameter *clone() const final;
  bool operator==(const Node &other) const final;
};

struct Function : public Node {

  std::string name;
  std::vector<std::shared_ptr<VarDecl>> parameters;
  std::shared_ptr<TypeExpr> return_type;
  std::vector<std::shared_ptr<Decl>> decls;
  std::vector<std::shared_ptr<Stmt>> body;

  Function() = delete;
  Function(const std::string &name_,
    std::vector<std::shared_ptr<VarDecl>> &&parameters_,
    std::shared_ptr<TypeExpr> return_type_,
    std::vector<std::shared_ptr<Decl>> &&decls_,
    std::vector<std::shared_ptr<Stmt>> &&body_, const location &loc_);
  Function(const Function &other);
  Function &operator=(Function other);
  friend void swap(Function &x, Function &y) noexcept;
  virtual ~Function() { }
  Function *clone() const final;
  bool operator==(const Node &other) const final;
  void validate() const final;
};

}
