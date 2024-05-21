#include <cstddef>
#include "options.h"
#include <string>
#include <cassert>
#include "codegen.h"
#include <rumur/rumur.h>
#include <vector>
#include <iostream>
#include <exception>

using namespace rumur;

namespace {

/// a visitor that prints SMV code
///
/// While `murphi2smv` only attempts to translate Models to SMV, this visitor is
/// actually capable of starting translation at a Node of any type.
class Printer : public ConstBaseTraversal {

private:
  std::ostream &o; ///< output stream to emit code to

  size_t indentation = 0; ///< current indentation level

  const std::vector<Comment> &comments; ///< comments from the original source file

  std::vector<bool> emitted;
  ///< whether each comment has been written to the output yet

  bool in_rule = false; ///< are we processing a non-start rule?

public:
  Printer(std::ostream &o_, const std::vector<Comment> &comments_): o{o_}, comments{comments_}, emitted(comments_.size(), false) {}

  void visit_add(const Add &n) final { *this << '(' << *n.lhs << " + " << *n.rhs << ')'; }

  void visit_aliasdecl(const AliasDecl &n) final { throw std::runtime_error("no"); }
  void visit_aliasrule(const AliasRule &n) final { throw std::runtime_error("no"); }
  void visit_aliasstmt(const AliasStmt &n) final { throw std::runtime_error("no"); }

  void visit_and(const And &n) final { *this << '(' << *n.lhs << " & " << *n.rhs << ')'; }

  void visit_array(const Array &n) final { throw std::runtime_error("no"); }

  void visit_assignment(const Assignment &n) final {
    // assume we are either within a startrule (in which case we need to use
    // `init` on variables on the LHS) or within another rule (in which case we
    // need to use `next` on variables on the LHS)
    *this << tab() << (in_rule ? "next" : "init") << '(' << *n.lhs << ") := " << *n.rhs << ";\n";
    }

  void visit_band(const Band &n) final { *this << '(' << *n.lhs << " & " << *n.rhs << ')'; }

  void visit_bnot(const Bnot &n) final { *this << '!' << *n.rhs; }

  void visit_bor(const Bor &n) final { *this << '(' << *n.lhs << " | " << *n.rhs << ')'; }

  void visit_clear(const Clear &n) final { throw std::runtime_error("no"); }

  void visit_constdecl(const ConstDecl &n) final {
    const Ptr<TypeExpr> type = n.get_type();
    *this << tab() << "FROZENVAR " << n.name << " : " << *type << ";\n";
    
    // constrain it to have exactly its known value
    *this << tab() << "INIT " << n.name << " == " << *n.value << ";\n";
    }

  void visit_div(const Div &n) final { *this << '(' << *n.lhs << " / " << *n.rhs << ')'; }

  void visit_element(const Element &n) final { *this << *n.array << '[' << *n.index << ']';}

  void visit_enum(const Enum &n) final { throw std::runtime_error("no"); }

  void visit_eq(const Eq &n) final { *this << '(' << *n.lhs << " = " << *n.rhs << ')'; }

  void visit_errorstmt(const ErrorStmt &n) final { throw std::runtime_error("no"); }
  void visit_exists(const Exists &n) final { throw std::runtime_error("no"); }

  void visit_exprid(const ExprID &n) final {
    if (n.is_literal_true()) {
      *this << "TRUE";
    } else if (n.is_literal_false()) {
      *this << "FALSE";
    } else {
      *this << n.id;
    }
  }

  void visit_field(const Field &n) final { throw std::runtime_error("no"); }
  void visit_for(const For &n) final { throw std::runtime_error("no"); }
  void visit_forall(const Forall &n) final { throw std::runtime_error("no"); }
  void visit_function(const Function &n) final { throw std::runtime_error("no"); }
  void visit_functioncall(const FunctionCall &n) final { throw std::runtime_error("no"); }

  void visit_geq(const Geq &n) final { *this << '(' << *n.lhs << " >= " << *n.rhs << ')'; }

  void visit_gt(const Gt &n) final { *this << '(' << *n.lhs << " > " << *n.rhs << ')'; }

  void visit_if(const If &n) final { throw std::runtime_error("no"); }
  void visit_ifclause(const IfClause &n) final { throw std::runtime_error("no"); }

  void visit_implication(const Implication &n) final { *this << '(' << *n.lhs << " -> " << *n.rhs << ')'; }

  void visit_isundefined(const IsUndefined &n) final { throw std::runtime_error("no"); }

  void visit_leq(const Leq &n) final { *this << '(' << *n.lhs << " <= " << *n.rhs << ')'; }

  void visit_lsh(const Lsh &n) final { *this << '(' << *n.lhs << " << " << *n.rhs << ')'; }

  void visit_lt(const Lt &n) final { *this << '(' << *n.lhs << " < " << *n.rhs << ')'; }

  void visit_model(const Model &n) final {
    emit_leading_comments(n);

    // force more natural placement for file-leading comments
    if (!n.children.empty())
      emit_leading_comments(*n.children[0]);

    // output module header
    *this << "MODULE " << module_name << '\n';
    indent();

    for (const Ptr<Node> &c : n.children) {
      emit_leading_comments(*c);
      *this << *c;
    }

    dedent();
    }

  void visit_mod(const Mod &n) final { *this << '(' << *n.lhs << " mod " << *n.rhs << ')'; }

  void visit_mul(const Mul &n) final { *this << '(' << *n.lhs << " * " << *n.rhs << ')'; }

  void visit_negative(const Negative &n) final { *this << '-' << *n.rhs; }

  void visit_neq(const Neq &n) final { *this << '(' << *n.lhs << " != " << *n.rhs << ')'; }

  void visit_not(const Not &n) final { *this << '!' << *n.rhs;}

  void visit_number(const Number &n) final { 
    if (numeric_type == "word") {
      if (n.value < 0) {
        *this << "-0sd_" << mpz_class{-n.value}.get_str();
      } else {
        *this << "0ud_" << n.value.get_str();
      }
    } else {
    *this << n.value.get_str();
    }
    }

  void visit_or(const Or &n) final { *this << '(' << *n.lhs << " | " << *n.rhs << ')'; }

  void visit_procedurecall(const ProcedureCall &n) final { throw std::runtime_error("no"); }
  void visit_property(const Property &n) final { throw std::runtime_error("no"); }
  void visit_propertyrule(const PropertyRule &n) final { throw std::runtime_error("no"); }
  void visit_propertystmt(const PropertyStmt &n) final { throw std::runtime_error("no"); }
  void visit_put(const Put &n) final { throw std::runtime_error("no"); }
  void visit_quantifier(const Quantifier &n) final { throw std::runtime_error("no"); }

  void visit_range(const Range &n) final {
    *this << *n.min << ".." << *n.max;
    }

  void visit_record(const Record &n) final { throw std::runtime_error("no"); }
  void visit_return(const Return &n) final { throw std::runtime_error("no"); }

  void visit_rsh(const Rsh &n) final { *this << '(' << *n.lhs << " >> " << *n.rhs << ')'; }

  void visit_ruleset(const Ruleset &n) final { throw std::runtime_error("no"); }
  void visit_scalarset(const Scalarset &n) final { throw std::runtime_error("no"); }

  void visit_simplerule(const SimpleRule &n) final {
    // TODO: containing params, guard, vars, decls
    
    const bool saved_in_rule = in_rule;
    in_rule = true;

    for (const Ptr<Stmt> &s : n.body) {
      emit_leading_comments(*s);
      *this << *s;
    }

    in_rule = saved_in_rule;
    }

  void visit_startstate(const StartState &n) final {
    // TODO: containing params, vars, decls

    for (const Ptr<Stmt> &s : n.body) {
      emit_leading_comments(*s);
      *this << *s;
    }

    }

  void visit_sub(const Sub &n) final { *this << '(' << *n.lhs << " - " << *n.rhs << ')'; }

  void visit_switch(const Switch &n) final { throw std::runtime_error("no"); }
  void visit_switchcase(const SwitchCase &n) final { throw std::runtime_error("no"); }

  void visit_ternary(const Ternary &n) final { *this << '(' << *n.cond << " ? " << *n.lhs << " : " << *n.rhs << ')'; }

  void visit_typedecl(const TypeDecl &n) final { throw std::runtime_error("no"); }

  void visit_typeexprid(const TypeExprID &n) final {
    // no need for special “boolean” handling because it has the same
    // spelling in Murphi and SMV
    *this << n.name;
  }

  void visit_undefine(const Undefine &n) final { throw std::runtime_error("no"); }

  void visit_vardecl(const VarDecl &n) final {
    *this << tab() << "VAR " << n.name << " : " << *n.get_type() << ";\n";
    }

  void visit_while(const While &n) final { throw std::runtime_error("no"); }

  void visit_xor(const Xor &n) final { *this << '(' << *n.lhs << " xor " << *n.rhs << ')';}

private:
  // wrappers to allow more readable code above
  Printer &operator<<(const std::string &s) {
    o << s;
    return *this;
  }
  Printer &operator<<(char c) {
    o << c;
    return *this;
  }
  Printer &operator<<(const Node &n) {
    n.visit(*this);
    return *this;
  }

  void indent() { ++indentation; }

  void dedent() {
    assert(indentation > 0);
    --indentation;
  }

  std::string tab() { return std::string(indentation * 2, ' '); }

  /// print any source comments to prior to the given node that have not yet
  /// been printed
  size_t emit_leading_comments(const Node &n) {
    size_t count = 0;
    size_t i = 0;
    for (const Comment &c : comments) {
      // has this not yet been printed?
      if (!emitted[i]) {
        // does this precede the given node?
        if (c.loc.end.line < n.loc.begin.line ||
            (c.loc.end.line == n.loc.begin.line &&
             c.loc.end.column <= n.loc.begin.column)) {

          // do some white space adjustment for multiline comments
          if (c.multiline) {
            o << tab() << "/--";
            bool dropping = false;
            for (const char &b : c.content) {
              if (b == '\n') {
                o << '\n' << tab() << ' ';
                dropping = true;
              } else if (dropping) {
                if (!isspace(b)) {
                  // account for “ *…” pattern of continuing multiline comments
                  if (b == '*') {
                    o << "--";
                    // this could end a comment if “/” is next, but we know “*/”
                    // cannot appear inside a Murphi multiline comment
                  } else {
                    o << b;
                  }
                  dropping = false;
                }
              } else {
                o << b;
              }
            }
            o << "--/\n";

          } else { // single line comments can be emitted simpler
            o << tab() << "--" << c.content << '\n';
          }

          emitted[i] = true;
        }
      }
      ++i;
    }
    return count;
  }
};

}

void codegen(const Node &n, const std::vector<Comment> &comments, std::ostream &out) {
  Printer p{out, comments};
  n.visit(p);
}
