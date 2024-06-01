#include "codegen.h"
#include "options.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <rumur/rumur.h>
#include <string>
#include <vector>

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

  const std::vector<Comment>
      &comments; ///< comments from the original source file

  std::vector<bool> emitted;
  ///< whether each comment has been written to the output yet

  bool in_rule = false; ///< are we processing a non-start rule?

  /// type declarations we have seen earlier
  std::vector<const TypeDecl *> typedecls;

  /// last member of `typedecls` we can look at to resolve references
  ///
  /// This is used to prevent expressions referencing later-defined type
  /// declarations. This should not happen, but in the case of shadowing type
  /// declarations this can induce a situation that would otherwise confuse this
  /// translator.
  size_t typedecls_watermark = 0;

public:
  Printer(std::ostream &o_, const std::vector<Comment> &comments_)
      : o{o_}, comments{comments_}, emitted(comments_.size(), false) {}

  void visit_add(const Add &n) final {
    *this << '(' << *n.lhs << " + " << *n.rhs << ')';
  }

  void visit_aliasdecl(const AliasDecl &n) final {
    *this << tab()
          << "/-- FIXME: Murphi alias declarations have no equivalent in SMV, "
          << n.name << " --/\n";
  }

  void visit_aliasrule(const AliasRule &n) final {
    *this << tab()
          << "/-- FIXME: Murphi alias rules have no equivalent in SMV --/\n";

    indent();
    for (const Ptr<Rule> &r : n.rules) {
      emit_leading_comments(*r);
      *this << *r;
    }
    dedent();
    *this << tab() << "/-- FIXME: end of alias rule --/\n";
  }

  void visit_aliasstmt(const AliasStmt &n) final {
    *this
        << tab()
        << "/-- FIXME: Murphi alias statements have no equivalent in SMV --/\n";

    indent();
    for (const Ptr<Stmt> &s : n.body) {
      emit_leading_comments(*s);
      *this << *s;
    }
    dedent();
    *this << tab() << "/-- FIXME: end of alias statement --/\n";
  }

  void visit_and(const And &n) final {
    *this << '(' << *n.lhs << " & " << *n.rhs << ')';
  }

  void visit_array(const Array &n) final {
    *this << "array " << *n.index_type << " of " << *n.element_type;
  }

  void visit_assignment(const Assignment &n) final {
    // assume we are either within a startrule (in which case we need to use
    // `init` on variables on the LHS) or within another rule (in which case we
    // need to use `next` on variables on the LHS)
    *this << tab() << (in_rule ? "next" : "init") << '(' << *n.lhs
          << ") := " << *n.rhs << ";\n";
  }

  void visit_band(const Band &n) final {
    *this << '(' << *n.lhs << " & " << *n.rhs << ')';
  }

  void visit_bnot(const Bnot &n) final { *this << '!' << *n.rhs; }

  void visit_bor(const Bor &n) final {
    *this << '(' << *n.lhs << " | " << *n.rhs << ')';
  }

  void visit_clear(const Clear &) final {
    *this
        << tab()
        << "/-- FIXME: Murphi clear statements have no equivalent in SMV --/\n";
  }

  void visit_constdecl(const ConstDecl &n) final {
    const Ptr<TypeExpr> type = n.get_type();
    *this << tab() << "FROZENVAR " << n.name << " : " << *type << ";\n";

    // constrain it to have exactly its known value
    *this << tab() << "INIT " << n.name << " == " << *n.value << ";\n";
  }

  void visit_div(const Div &n) final {
    *this << '(' << *n.lhs << " / " << *n.rhs << ')';
  }

  void visit_element(const Element &n) final {
    *this << *n.array << '[' << *n.index << ']';
  }

  void visit_enum(const Enum &n) final {
    *this << '{';
    const char *separator = "";
    for (const std::pair<std::string, location> &m : n.members) {
      *this << separator << m.first;
      separator = ", ";
    }
    *this << '}';
  }

  void visit_eq(const Eq &n) final {
    *this << '(' << *n.lhs << " = " << *n.rhs << ')';
  }

  void visit_errorstmt(const ErrorStmt &) final {
    *this
        << tab()
        << "/-- FIXME: Murphi error statements have no equivalent in SMV --/\n";
  }

  void visit_exists(const Exists &) final {
    *this
        << "/-- FIXME: Murphi exists expressions have no equivalent in SMV --/";
  }

  void visit_exprid(const ExprID &n) final {
    if (n.is_literal_true()) {
      *this << "TRUE";
    } else if (n.is_literal_false()) {
      *this << "FALSE";
    } else {
      *this << n.id;
    }
  }

  void visit_field(const Field &n) final {
    *this << *n.record << "." << n.field;
  }

  void visit_for(const For &n) final {
    *this << tab()
          << "/-- FIXME: Murphi for loops have no equivalent in SMV --/\n";

    indent();
    for (const Ptr<Stmt> &s : n.body) {
      emit_leading_comments(*s);
      *this << *s;
    }
    dedent();
    *this << tab() << "/-- FIXME: end of for loop --/\n";
  }

  void visit_forall(const Forall &) final {
    *this
        << "/-- FIXME: Murphi forall expressions have no equivalent in SMV --/";
  }

  void visit_function(const Function &n) final {
    *this << tab()
          << "/-- FIXME: Murphi functions have no equivalent in SMV --/\n";
    indent();
    for (const Ptr<Decl> &d : n.decls) {
      emit_leading_comments(*d);
      *this << *d;
    }
    for (const Ptr<Stmt> &s : n.body) {
      emit_leading_comments(*s);
      *this << *s;
    }
    dedent();
    *this << tab() << "/-- FIXME: end of function --/\n";
  }

  void visit_functioncall(const FunctionCall &n) final {
    *this << "/-- FIXME: Murphi function calls have no equivalent in SMV, "
          << n.name << " --/";
  }

  void visit_geq(const Geq &n) final {
    *this << '(' << *n.lhs << " >= " << *n.rhs << ')';
  }

  void visit_gt(const Gt &n) final {
    *this << '(' << *n.lhs << " > " << *n.rhs << ')';
  }

  void visit_if(const If &n) final {
    const char *separator = "";
    for (const IfClause &c : n.clauses) {
      *this << tab()
            << "/-- FIXME: Murphi if statements have no equivalent in SMV, `"
            << separator << c;

      separator = "else ";
    }
  }

  void visit_ifclause(const IfClause &n) final {
    if (n.condition != nullptr)
      *this << "if " << *n.condition;
    *this << "` --/\n";
    indent();
    for (const Ptr<Stmt> &s : n.body) {
      emit_leading_comments(*s);
      *this << *s;
    }
    dedent();
  }

  void visit_implication(const Implication &n) final {
    *this << '(' << *n.lhs << " -> " << *n.rhs << ')';
  }

  void visit_isundefined(const IsUndefined &n) final {
    *this << tab()
          << "/-- FIXME: Murphi isundefined statements have no equivalent in "
             "SMV, `"
          << *n.rhs << "` --/";
  }

  void visit_leq(const Leq &n) final {
    *this << '(' << *n.lhs << " <= " << *n.rhs << ')';
  }

  void visit_lsh(const Lsh &n) final {
    *this << '(' << *n.lhs << " << " << *n.rhs << ')';
  }

  void visit_lt(const Lt &n) final {
    *this << '(' << *n.lhs << " < " << *n.rhs << ')';
  }

  void visit_model(const Model &n) final {
    emit_leading_comments(n);

    // force more natural placement for file-leading comments
    if (!n.children.empty())
      emit_leading_comments(*n.children[0]);

    // output module header
    *this << "MODULE main\n";
    indent();

    for (const Ptr<Node> &c : n.children) {
      emit_leading_comments(*c);
      *this << *c;
    }

    dedent();
  }

  void visit_mod(const Mod &n) final {
    *this << '(' << *n.lhs << " mod " << *n.rhs << ')';
  }

  void visit_mul(const Mul &n) final {
    *this << '(' << *n.lhs << " * " << *n.rhs << ')';
  }

  void visit_negative(const Negative &n) final { *this << '-' << *n.rhs; }

  void visit_neq(const Neq &n) final {
    *this << '(' << *n.lhs << " != " << *n.rhs << ')';
  }

  void visit_not(const Not &n) final { *this << '!' << *n.rhs; }

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

  void visit_or(const Or &n) final {
    *this << '(' << *n.lhs << " | " << *n.rhs << ')';
  }

  void visit_procedurecall(const ProcedureCall &n) final {
    *this << tab()
          << "/-- FIXME: Murphi procedure calls have no equivalent in SMV, "
          << n.call.name << " --/\n";
  }

  void visit_property(const Property &n) final {
    switch (n.category) {
    case Property::ASSERTION:
      *this << tab() << "INVAR " << *n.expr << ";\n";
      break;
    default:
      *this << tab()
            << "/-- FIXME: Murphi non-assertion properties have no equivalent "
               "in SMV, `"
            << *n.expr << "` --/\n";
      break;
    }
  }

  void visit_propertyrule(const PropertyRule &n) final {
    if (n.name != "")
      *this << tab() << "-- property " << n.name << '\n';
    *this << n.property;
  }

  void visit_propertystmt(const PropertyStmt &) final {
    *this << tab()
          << "/-- FIXME: Murphi property statements have no equivalent in SMV "
             "--/\n";
  }

  void visit_put(const Put &n) final {
    *this << tab()
          << "/-- FIXME: Murphi put statements have no equivalent in SMV --/ ";
    if (n.expr == nullptr) {
      *this << '"' << n.value << '"';
    } else {
      *this << *n.expr;
    }
    *this << ";\n";
  }

  void visit_quantifier(const Quantifier &) final {
    *this << "/-- FIXME: Murphi quantifiers have no equivalent in SMV --/";
  }

  void visit_range(const Range &n) final { *this << *n.min << ".." << *n.max; }

  void visit_record(const Record &n) final {
    *this << "MODULE\n";
    indent();
    *this << tab()
          << "/-- FIXME: this will need to be refactored to be named and "
             "precede `main` --/\n";

    for (const Ptr<VarDecl> &f : n.fields) {
      emit_leading_comments(*f);
      *this << *f;
    }
    *this << tab();

    dedent();
  }

  void visit_return(const Return &n) final {
    *this
        << tab()
        << "/-- FIXME: Murphi return statements have no equivalent in SMV --/";
    if (n.expr != nullptr)
      *this << ' ' << *n.expr << ';';
    *this << '\n';
  }

  void visit_rsh(const Rsh &n) final {
    *this << '(' << *n.lhs << " >> " << *n.rhs << ')';
  }

  void visit_ruleset(const Ruleset &n) final {
    *this << tab()
          << "/-- FIXME: Murphi rulesets have no equivalent in SMV --/\n";

    indent();
    for (const Ptr<Rule> &r : n.rules) {
      emit_leading_comments(*r);
      *this << *r;
    }
    dedent();
    *this << tab() << "/-- FIXME: end of ruleset --/\n";
  }

  void visit_scalarset(const Scalarset &n) final {
    if (numeric_type == "word") {
      *this << "0ud_0";
    } else {
      *this << "0";
    }
    *this << "..(" << *n.bound << " - ";
    if (numeric_type == "word") {
      *this << "0ud_1";
    } else {
      *this << "1";
    }
  }

  void visit_simplerule(const SimpleRule &n) final {

    *this << tab() << "/-- FIXME: Murphi rules have no equivalent in SMV --/\n";
    if (n.guard != nullptr)
      *this << tab() << "/-- FIXME: guard, `" << *n.guard << "` --/\n";

    if (!n.decls.empty()) {
      *this << tab()
            << "/-- FIXME: these rule declarations will need to be lifted to "
               "globals --/\n";
      indent();
      for (const Ptr<Decl> &d : n.decls) {
        emit_leading_comments(*d);
        *this << *d;
      }
      dedent();
    }
    const bool saved_in_rule = in_rule;
    in_rule = true;

    indent();
    for (const Ptr<Stmt> &s : n.body) {
      emit_leading_comments(*s);
      *this << *s;
    }
    dedent();
    *this << tab() << "/-- FIXME: end of rule --/\n";

    in_rule = saved_in_rule;
  }

  void visit_startstate(const StartState &n) final {

    if (!n.decls.empty()) {
      *this << tab()
            << "/-- FIXME: these startstate declarations will need to be "
               "lifted to globals --/\n";
      indent();
      for (const Ptr<Decl> &d : n.decls) {
        emit_leading_comments(*d);
        *this << *d;
      }
      dedent();
    }

    for (const Ptr<Stmt> &s : n.body) {
      emit_leading_comments(*s);
      *this << *s;
    }
  }

  void visit_sub(const Sub &n) final {
    *this << '(' << *n.lhs << " - " << *n.rhs << ')';
  }

  void visit_switch(const Switch &n) final {
    *this << tab() << "/-- FIXME: Murphi switches have no equivalent in SMV, `"
          << *n.expr << "` --/\n";

    indent();
    for (const SwitchCase &c : n.cases) {
      emit_leading_comments(c);
      *this << c;
    }
    dedent();
    *this << tab() << "/-- FIXME: end of switch --/\n";
  }

  void visit_switchcase(const SwitchCase &n) final {
    *this << tab() << "/-- switch case matches ";
    const char *separator = "";
    for (const Ptr<Expr> &m : n.matches) {
      *this << separator << '`' << *m << '`';
      separator = ", ";
    }
    *this << " --/\n";

    indent();
    for (const Ptr<Stmt> &s : n.body) {
      emit_leading_comments(*s);
      *this << *s;
    }
    dedent();
  }

  void visit_ternary(const Ternary &n) final {
    *this << '(' << *n.cond << " ? " << *n.lhs << " : " << *n.rhs << ')';
  }

  void visit_typedecl(const TypeDecl &n) final {
    // there is no SMV equivalent, so memorise this for later inline expansion
    typedecls.push_back(&n);
    assert(typedecls_watermark == typedecls.size() - 1);
    ++typedecls_watermark;
  }

  void visit_typeexprid(const TypeExprID &n) final {
    // this may be a reference to a typedecl we have previously seen
    assert(typedecls_watermark <= typedecls.size());
    for (size_t watermark = typedecls_watermark - 1; watermark != SIZE_MAX;
         --watermark) {
      const TypeDecl *candidate = typedecls[watermark];
      if (n.name == candidate->name) {
        const size_t saved_watermark = typedecls_watermark;
        typedecls_watermark = watermark;
        *this << *candidate->value;
        typedecls_watermark = saved_watermark;
        return;
      }
    }

    // no need for special “boolean” handling because it has the same
    // spelling in Murphi and SMV
    *this << n.name;
  }

  void visit_undefine(const Undefine &n) final {
    *this
        << tab()
        << "/-- FIXME: Murphi undefine statements have no equivalent in SMV, `"
        << *n.rhs << "` --/\n";
  }

  void visit_vardecl(const VarDecl &n) final {
    *this << tab() << "VAR " << n.name << " : " << *n.get_type() << ";\n";
  }

  void visit_while(const While &n) final {
    *this << tab()
          << "/-- FIXME: Murphi while statements have no equivalent in SMV, `"
          << *n.condition << "` --/\n";

    indent();
    for (const Ptr<Stmt> &s : n.body) {
      emit_leading_comments(*s);
      *this << *s;
    }
    dedent();
    *this << tab() << "/-- FIXME: end of while --/\n";
  }

  void visit_xor(const Xor &n) final {
    *this << '(' << *n.lhs << " xor " << *n.rhs << ')';
  }

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

} // namespace

void codegen(const Node &n, const std::vector<Comment> &comments,
             std::ostream &out) {
  Printer p{out, comments};
  n.visit(p);
}
