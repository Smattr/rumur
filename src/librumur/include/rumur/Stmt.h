#pragma once

#include <iostream>
#include "location.hh"
#include <rumur/Expr.h>
#include <rumur/Node.h>
#include <string>

namespace rumur {

class Stmt : public Node {

 public:
  using Node::Node;

  Stmt() = delete;
  Stmt(const Stmt&) = default;
  Stmt(Stmt&&) = default;
  Stmt &operator=(const Stmt&) = default;
  Stmt &operator=(Stmt&&) = default;
  virtual ~Stmt() { }
  virtual Stmt *clone() const = 0;

};

class Assignment : public Stmt {

 public:
  Lvalue *lhs;
  Expr *rhs;

  Assignment() = delete;
  Assignment(Lvalue *lhs_, Expr *rhs_, const location &loc_);
  Assignment(const Assignment &other);
  Assignment &operator=(Assignment other);
  friend void swap(Assignment &x, Assignment &y) noexcept;
  Assignment *clone() const final;
  virtual ~Assignment();

  void validate() const final;

  void generate(std::ostream &out) const final;

};

class Error : public Stmt {

  public:
   std::string message;

   Error() = delete;
   Error(const std::string &message_, const location &loc_);
   Error(const Error &other);
   Error &operator=(Error other);
   friend void swap(Error &x, Error &y) noexcept;
   Error *clone() const final;
   virtual ~Error() { }

   void validate() const final;

   void generate(std::ostream &out) const final;
};

}
