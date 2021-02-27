#include "XMLPrinter.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <rumur/rumur.h>
#include <string>

using namespace rumur;

static std::string xml_escape(char c) {
  switch (c) {
    case '"' : return "&quot;";
    case '\'': return "&apos;";
    case '<' : return "&lt;";
    case '>' : return "&gt;";
    case '&' : return "&amp;";

    /* XXX: Form feed is apparently not a valid character to use in XML 1.0,
     * encoded or otherwise. However, some legacy models use this. To cope with
     * it, we just translate it to a single space.
     */
    case 12  : return " ";

    default  : return std::string(1, c);
  }
}

static std::string xml_escape(const std::string &s) {
  std::string escaped;

  for (char c : s)
    escaped += xml_escape(c);

  return escaped;
}

XMLPrinter::XMLPrinter(const std::string &in_filename, std::istream &in_,
                       std::ostream &o_)
    : in(in_), o(o_) {

  // Write out XML version header
  o << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";

  o << "<unit filename=\"" << xml_escape(in_filename) << "\">";
}

void XMLPrinter::visit_add(const Add &n) { visit_bexpr("add", n); }

void XMLPrinter::visit_aliasdecl(const AliasDecl &n) {
  sync_to(n);
  o << "<aliasdecl name=\"" << n.name << "\" ";
  add_location(n);
  o << ">";
  sync_to(*n.value);
  o << "<value>";
  dispatch(*n.value);
  o << "</value>";
  sync_to(n.loc.end);
  o << "</aliasdecl>";
}

void XMLPrinter::visit_aliasrule(const AliasRule &n) {
  sync_to(n);
  o << "<aliasrule name=\"" << xml_escape(n.name) << "\" ";
  add_location(n);
  o << ">";
  if (!n.aliases.empty()) {
    sync_to(*n.aliases[0]);
    o << "<aliases>";
    for (auto &a : n.aliases) {
      sync_to(*a);
      dispatch(*a);
    }
    o << "</aliases>";
  }
  if (!n.rules.empty()) {
    sync_to(*n.rules[0]);
    o << "<rules>";
    for (auto &r : n.rules) {
      sync_to(*r);
      dispatch(*r);
    }
    o << "</rules>";
  }
  sync_to(n.loc.end);
  o << "</aliasrule>";
}

void XMLPrinter::visit_aliasstmt(const AliasStmt &n) {
  sync_to(n);
  o << "<aliasstmt ";
  add_location(n);
  o << ">";
  if (!n.aliases.empty()) {
    sync_to(*n.aliases[0]);
    o << "<aliases>";
    for (auto &a : n.aliases) {
      sync_to(*a);
      dispatch(*a);
    }
    o << "</aliases>";
  }
  if (!n.body.empty()) {
    sync_to(*n.body[0]);
    o << "<body>";
    for (auto &s : n.body) {
      sync_to(*s);
      dispatch(*s);
    }
    o << "</body>";
  }
  sync_to(n.loc.end);
  o << "</aliasstmt>";
}

void XMLPrinter::visit_and(const And &n) { visit_bexpr("and", n); }

void XMLPrinter::visit_array(const Array &n) {
  sync_to(n);
  o << "<array ";
  add_location(n);
  o << ">";
  sync_to(*n.index_type);
  o << "<indextype>";
  dispatch(*n.index_type);
  o << "</indextype>";
  sync_to(*n.element_type);
  o << "<elementtype>";
  dispatch(*n.element_type);
  o << "</elementtype>";
  sync_to(n.loc.end);
  o << "</array>";
}

void XMLPrinter::visit_assignment(const Assignment &n) {
  sync_to(n);
  o << "<assignment ";
  add_location(n);
  o << "><lhs>";
  dispatch(*n.lhs);
  o << "</lhs>";
  sync_to(*n.rhs);
  o << "<rhs>";
  dispatch(*n.rhs);
  o << "</rhs>";
  sync_to(n.loc.end);
  o << "</assignment>";
}

void XMLPrinter::visit_band(const Band &n) { visit_bexpr("band", n); }

void XMLPrinter::visit_bnot(const Bnot &n) { visit_uexpr("bnot", n); }

void XMLPrinter::visit_bor(const Bor &n) { visit_bexpr("bor", n); }

void XMLPrinter::visit_clear(const Clear &n) {
  sync_to(n);
  o << "<clear ";
  add_location(n);
  o << ">";
  sync_to(*n.rhs);
  dispatch(*n.rhs);
  sync_to(n.loc.end);
  o << "</clear>";
}

void XMLPrinter::visit_constdecl(const ConstDecl &n) {
  sync_to(n);
  o << "<constdecl name=\"" << n.name << "\" ";
  add_location(n);
  o << ">";
  sync_to(*n.value);
  o << "<value>";
  dispatch(*n.value);
  o << "</value>";
  sync_to(n.loc.end);
  o << "</constdecl>";
}

void XMLPrinter::visit_div(const Div &n) { visit_bexpr("div", n); }

void XMLPrinter::visit_element(const Element &n) {
  sync_to(n);
  o << "<element ";
  add_location(n);
  o << ">";
  sync_to(*n.array);
  o << "<lhs>";
  dispatch(*n.array);
  o << "</lhs>";
  sync_to(*n.index);
  o << "<rhs>";
  dispatch(*n.index);
  o << "</rhs>";
  sync_to(n.loc.end);
  o << "</element>";
}

void XMLPrinter::visit_enum(const Enum &n) {
  sync_to(n);
  o << "<enum ";
  add_location(n);
  o << ">";
  for (const std::pair<std::string, location> &m : n.members) {
    sync_to(m.second.begin);
    o << "<member name=\"" << m.first << "\" "
      << "first_line=\"" << m.second.begin.line << "\" "
      << "first_column=\"" << m.second.begin.column << "\" "
      << "last_line=\"" << m.second.end.line << "\" "
      << "last_column=\"" << m.second.end.column << "\">";
    sync_to(m.second.end);
    o << "</member>";
  }
  sync_to(n.loc.end);
  o << "</enum>";
}

void XMLPrinter::visit_eq(const Eq &n) { visit_bexpr("eq", n); }

void XMLPrinter::visit_errorstmt(const ErrorStmt &n) {
  sync_to(n);
  o << "<errorstmt message=\"" << xml_escape(n.message) << "\" ";
  add_location(n);
  o << ">";
  sync_to(n.loc.end);
  o << "</errorstmt>";
}

void XMLPrinter::visit_exists(const Exists &n) {
  sync_to(n);
  o << "<exists ";
  add_location(n);
  o << ">";
  sync_to(n.quantifier);
  o << "<quan>";
  dispatch(n.quantifier);
  o << "</quan>";
  sync_to(*n.expr);
  o << "<expr>";
  dispatch(*n.expr);
  o << "</expr>";
  sync_to(n.loc.end);
  o << "</exists>";
}

void XMLPrinter::visit_exprid(const ExprID &n) {
  sync_to(n);
  o << "<exprid id=\"" << n.id << "\" ";
  add_location(n);
  /* We deliberately omit printing n.value because this is the declaration we
   * discovered that this ID points to during symbol lookup. I.e. n.value is not
   * a "child" of this node in the sense of the source.
   */
  o << ">";
  sync_to(n.loc.end);
  o << "</exprid>";
}

void XMLPrinter::visit_field(const Field &n) {
  sync_to(n);
  o << "<field ";
  add_location(n);
  o << ">";
  sync_to(*n.record);
  o << "<lhs>";
  dispatch(*n.record);
  o << "</lhs><rhs><string value=\"" << n.field << "\">";
  /* FIXME: We don't have location information for the field itself, so we just
   * dump the entire remaining text of this node here. Does this produce
   * inaccurate output?
   */
  sync_to(n.loc.end);
  o << "</string></rhs></field>";
}

void XMLPrinter::visit_for(const For &n) {
  sync_to(n);
  o << "<forstmt ";
  add_location(n);
  o << ">";
  sync_to(n.quantifier);
  dispatch(n.quantifier);
  if (!n.body.empty()) {
    sync_to(*n.body[0]);
    o << "<body>";
    for (auto &s : n.body) {
      sync_to(*s);
      dispatch(*s);
    }
    o << "</body>";
  }
  sync_to(n.loc.end);
  o << "</forstmt>";
}

void XMLPrinter::visit_forall(const Forall &n) {
  sync_to(n);
  o << "<forall ";
  add_location(n);
  o << ">";
  sync_to(n.quantifier);
  o << "<quan>";
  dispatch(n.quantifier);
  o << "</quan>";
  sync_to(*n.expr);
  o << "<expr>";
  dispatch(*n.expr);
  o << "</expr>";
  sync_to(n.loc.end);
  o << "</forall>";
}

void XMLPrinter::visit_function(const Function &n) {
  sync_to(n);
  o << "<function name=\"" << n.name << "\" ";
  add_location(n);
  o << ">";
  if (!n.parameters.empty()) {
    sync_to(*n.parameters[0]);
    o << "<parameters>";
    for (auto &p : n.parameters) {
      sync_to(*p);
      dispatch(*p);
    }
    o << "</parameters>";
  }
  if (n.return_type != nullptr) {
    sync_to(*n.return_type);
    o << "<returntype>";
    dispatch(*n.return_type);
    o << "</returntype>";
  }
  if (!n.decls.empty()) {
    sync_to(*n.decls[0]);
    o << "<decls>";
    for (auto &d : n.decls) {
      sync_to(*d);
      dispatch(*d);
    }
    o << "</decls>";
  }
  if (!n.body.empty()) {
    sync_to(*n.body[0]);
    o << "<body>";
    for (auto &s : n.body) {
      sync_to(*s);
      dispatch(*s);
    }
    o << "</body>";
  }
  sync_to(n.loc.end);
  o << "</function>";
}

void XMLPrinter::visit_functioncall(const FunctionCall &n) {
  sync_to(n);
  /* We deliberately list the called function by name as an attribute, rather
   * than emitting the function itself as a child of this node because morally
   * this is just a reference to a previously defined function.
   */
  o << "<functioncall name=\"" << n.name << "\" ";
  add_location(n);
  o << ">";
  for (auto &a : n.arguments) {
    sync_to(*a);
    o << "<argument>";
    dispatch(*a);
    o << "</argument>";
  }
  sync_to(n.loc.end);
  o << "</functioncall>";
}

void XMLPrinter::visit_geq(const Geq &n) { visit_bexpr("geq", n); }

void XMLPrinter::visit_gt(const Gt &n) { visit_bexpr("gt", n); }

void XMLPrinter::visit_if(const If &n) {
  sync_to(n);
  o << "<if ";
  add_location(n);
  o << ">";
  for (const IfClause &c : n.clauses) {
    sync_to(c);
    dispatch(c);
  }
  sync_to(n.loc.end);
  o << "</if>";
}

void XMLPrinter::visit_ifclause(const IfClause &n) {
  sync_to(n);
  o << "<ifclause ";
  add_location(n);
  o << ">";
  if (n.condition != nullptr) {
    sync_to(*n.condition);
    o << "<condition>";
    dispatch(*n.condition);
    o << "</condition>";
  }
  if (!n.body.empty()) {
    sync_to(*n.body[0]);
    o << "<body>";
    for (auto &s : n.body) {
      sync_to(*s);
      dispatch(*s);
    }
    o << "</body>";
  }
  sync_to(n.loc.end);
  o << "</ifclause>";
}

void XMLPrinter::visit_implication(const Implication &n) {
  visit_bexpr("implication", n);
}

void XMLPrinter::visit_isundefined(const IsUndefined &n) {
  visit_uexpr("isundefined", n);
}

void XMLPrinter::visit_leq(const Leq &n) { visit_bexpr("leq", n); }

void XMLPrinter::visit_lsh(const Lsh &n) { visit_bexpr("lsh", n); }

void XMLPrinter::visit_lt(const Lt &n) { visit_bexpr("lt", n); }

void XMLPrinter::visit_mod(const Mod &n) { visit_bexpr("mod", n); }

void XMLPrinter::visit_model(const Model &n) {
  o << "<model ";
  add_location(n);
  o << ">";
  if (!n.children.empty()) {
    sync_to(*n.children[0]);
    for (auto &c : n.children) {
      sync_to(*c);
      dispatch(*c);
    }
  }
  sync_to(n.loc.end);
  o << "</model>";
}

void XMLPrinter::visit_mul(const Mul &n) { visit_bexpr("mul", n); }

void XMLPrinter::visit_negative(const Negative &n) {
  visit_uexpr("negative", n);
}

void XMLPrinter::visit_neq(const Neq &n) { visit_bexpr("neq", n); }

void XMLPrinter::visit_not(const Not &n) { visit_uexpr("not", n); }

void XMLPrinter::visit_number(const Number &n) {
  sync_to(n);
  o << "<number value=\"" << n.value << "\" ";
  add_location(n);
  o << ">";
  sync_to(n.loc.end);
  o << "</number>";
}

void XMLPrinter::visit_or(const Or &n) { visit_bexpr("or", n); }

void XMLPrinter::visit_procedurecall(const ProcedureCall &n) {
  sync_to(n);
  o << "<procedurecall ";
  add_location(n);
  o << ">";
  sync_to(n.call);
  dispatch(n.call);
  sync_to(n.loc.end);
  o << "</procedurecall>";
}

void XMLPrinter::visit_property(const Property &n) {
  sync_to(n);
  o << "<property category=\"";
  switch (n.category) {
    case Property::ASSERTION:  o << "assertion";  break;
    case Property::ASSUMPTION: o << "assumption"; break;
    case Property::COVER:      o << "cover";      break;
    case Property::LIVENESS:   o << "liveness";   break;
  }
  o << "\" ";
  add_location(n);
  o << ">";
  sync_to(*n.expr);
  o << "<expr>";
  dispatch(*n.expr);
  o << "</expr>";
  sync_to(n.loc.end);
  o << "</property>";
}

void XMLPrinter::visit_propertyrule(const PropertyRule &n) {
  sync_to(n);
  o << "<propertyrule name=\"" << xml_escape(n.name) << "\" ";
  add_location(n);
  o << ">";
  if (!n.quantifiers.empty()) {
    sync_to(n.quantifiers[0]);
    o << "<quantifiers>";
    for (const Quantifier &q : n.quantifiers) {
      sync_to(q);
      dispatch(q);
    }
    o << "</quantifiers>";
  }
  sync_to(n.property);
  dispatch(n.property);
  sync_to(n.loc.end);
  o << "</propertyrule>";
}

void XMLPrinter::visit_propertystmt(const PropertyStmt &n) {
  sync_to(n);
  o << "<propertystmt message=\"" << xml_escape(n.message) << "\" ";
  add_location(n);
  o << ">";
  sync_to(n.property);
  dispatch(n.property);
  sync_to(n.loc.end);
  o << "</propertystmt>";
}

void XMLPrinter::visit_put(const Put &n) {
  sync_to(n);
  o << "<put ";
  if (n.expr == nullptr) {
    o << "value=\"" << xml_escape(n.value) << "\" ";
  }
  add_location(n);
  o << ">";
  if (n.expr != nullptr) {
    sync_to(*n.expr);
    dispatch(*n.expr);
  }
  sync_to(n.loc.end);
  o << "</put>";
}

void XMLPrinter::visit_quantifier(const Quantifier &n) {
  sync_to(n);
  o << "<quantifier name=\"" << xml_escape(n.name) << "\" ";
  add_location(n);
  o << ">";
  if (n.type != nullptr) {
    sync_to(*n.type);
    o << "<type>";
    dispatch(*n.type);
    o << "</type>";
  }
  if (n.from != nullptr) {
    sync_to(*n.from);
    o << "<from>";
    dispatch(*n.from);
    o << "</from>";
  }
  if (n.to != nullptr) {
    sync_to(*n.to);
    o << "<to>";
    dispatch(*n.to);
    o << "</to>";
  }
  if (n.step != nullptr) {
    sync_to(*n.step);
    o << "<step>";
    dispatch(*n.step);
    o << "</step>";
  }
  sync_to(n.loc.end);
  o << "</quantifier>";
}

void XMLPrinter::visit_range(const Range &n) {
  sync_to(n);
  o << "<range ";
  add_location(n);
  o << ">";
  sync_to(*n.min);
  o << "<min>";
  dispatch(*n.min);
  o << "</min>";
  sync_to(*n.max);
  o << "<max>";
  dispatch(*n.max);
  o << "</max>";
  sync_to(n.loc.end);
  o << "</range>";
}

void XMLPrinter::visit_record(const Record &n) {
  sync_to(n);
  o << "<record ";
  add_location(n);
  o << ">";
  for (auto &f : n.fields) {
    sync_to(*f);
    dispatch(*f);
  }
  sync_to(n.loc.end);
  o << "</record>";
}

void XMLPrinter::visit_return(const Return &n) {
  sync_to(n);
  o << "<return ";
  add_location(n);
  o << ">";
  if (n.expr != nullptr) {
    sync_to(*n.expr);
    dispatch(*n.expr);
  }
  sync_to(n.loc.end);
  o << "</return>";
}

void XMLPrinter::visit_rsh(const Rsh &n) { visit_bexpr("rsh", n); }

void XMLPrinter::visit_ruleset(const Ruleset &n) {
  sync_to(n);
  o << "<ruleset name=\"" << xml_escape(n.name) << "\" ";
  add_location(n);
  o << ">";
  if (!n.quantifiers.empty()) {
    sync_to(n.quantifiers[0]);
    o << "<quantifiers>";
    for (const Quantifier &q : n.quantifiers) {
      sync_to(q);
      dispatch(q);
    }
    o << "</quantifiers>";
  }
  if (!n.rules.empty()) {
    sync_to(*n.rules[0]);
    o << "<rules>";
    for (auto &r : n.rules) {
      sync_to(*r);
      dispatch(*r);
    }
    o << "</rules>";
  }
  sync_to(n.loc.end);
  o << "</ruleset>";
}

void XMLPrinter::visit_scalarset(const Scalarset &n) {
  sync_to(n);
  o << "<scalarset ";
  add_location(n);
  o << ">";
  sync_to(*n.bound);
  o << "<bound>";
  dispatch(*n.bound);
  o << "</bound>";
  sync_to(n.loc.end);
  o << "</scalarset>";
}

void XMLPrinter::visit_simplerule(const SimpleRule &n) {
  sync_to(n);
  o << "<simplerule name=\"" << xml_escape(n.name) << "\" ";
  add_location(n);
  o << ">";
  if (!n.quantifiers.empty()) {
    sync_to(n.quantifiers[0]);
    o << "<quantifiers>";
    for (const Quantifier &q : n.quantifiers) {
      sync_to(q);
      dispatch(q);
    }
    o << "</quantifiers>";
  }
  if (n.guard != nullptr) {
    sync_to(*n.guard);
    o << "<guard>";
    dispatch(*n.guard);
    o << "</guard>";
  }
  if (!n.decls.empty()) {
    sync_to(*n.decls[0]);
    o << "<decls>";
    for (auto &d : n.decls) {
      sync_to(*d);
      dispatch(*d);
    }
    o << "</decls>";
  }
  if (!n.body.empty()) {
    sync_to(*n.body[0]);
    o << "<body>";
    for (auto &s : n.body) {
      sync_to(*s);
      dispatch(*s);
    }
    o << "</body>";
  }
  sync_to(n.loc.end);
  o << "</simplerule>";
}

void XMLPrinter::visit_startstate(const StartState &n) {
  sync_to(n);
  o << "<startstate name=\"" << xml_escape(n.name) << "\" ";
  add_location(n);
  o << ">";
  if (!n.quantifiers.empty()) {
    sync_to(n.quantifiers[0]);
    o << "<quantifiers>";
    for (const Quantifier &q : n.quantifiers) {
      sync_to(q);
      dispatch(q);
    }
    o << "</quantifiers>";
  }
  if (!n.decls.empty()) {
    sync_to(*n.decls[0]);
    o << "<decls>";
    for (auto &d : n.decls) {
      sync_to(*d);
      dispatch(*d);
    }
    o << "</decls>";
  }
  if (!n.body.empty()) {
    sync_to(*n.body[0]);
    o << "<body>";
    for (auto &s : n.body) {
      sync_to(*s);
      dispatch(*s);
    }
    o << "</body>";
  }
  sync_to(n.loc.end);
  o << "</startstate>";
}

void XMLPrinter::visit_sub(const Sub &n) { visit_bexpr("sub", n); }

void XMLPrinter::visit_switch(const Switch &n) {
  sync_to(n);
  o << "<switch ";
  add_location(n);
  o << ">";
  sync_to(*n.expr);
  o << "<expr>";
  dispatch(*n.expr);
  o << "</expr>";
  if (!n.cases.empty()) {
    sync_to(n.cases[0]);
    o << "<cases>";
    for (const SwitchCase &c : n.cases) {
      sync_to(c);
      dispatch(c);
    }
    o << "</cases>";
  }
  sync_to(n.loc.end);
  o << "</switch>";
}

void XMLPrinter::visit_switchcase(const SwitchCase &n) {
  sync_to(n);
  o << "<case>";
  if (!n.matches.empty()) {
    sync_to(*n.matches[0]);
    o << "<matches>";
    for (auto &m : n.matches) {
      sync_to(*m);
      dispatch(*m);
    }
    o << "</matches>";
  }
  if (!n.body.empty()) {
    sync_to(*n.body[0]);
    o << "<body>";
    for (auto &s : n.body) {
      sync_to(*s);
      dispatch(*s);
    }
    o << "</body>";
  }
  sync_to(n.loc.end);
  o << "</case>";
}

void XMLPrinter::visit_ternary(const Ternary &n) {
  sync_to(n);
  o << "<ternary ";
  add_location(n);
  o << ">";
  sync_to(*n.cond);
  o << "<condition>";
  dispatch(*n.cond);
  o << "</condition>";
  sync_to(*n.lhs);
  o << "<lhs>";
  dispatch(*n.lhs);
  o << "</lhs>";
  sync_to(*n.rhs);
  o << "<rhs>";
  dispatch(*n.rhs);
  o << "</rhs>";
  sync_to(n.loc.end);
  o << "</ternary>";
}

void XMLPrinter::visit_typedecl(const TypeDecl &n) {
  sync_to(n);
  o << "<typedecl name=\"" << n.name << "\" ";
  add_location(n);
  o << ">";
  sync_to(*n.value);
  o << "<value>";
  dispatch(*n.value);
  o << "</value>";
  sync_to(n.loc.end);
  o << "</typedecl>";
}

void XMLPrinter::visit_typeexprid(const TypeExprID &n) {
  sync_to(n);
  o << "<typeexprid name=\"" << n.name << "\" ";
  add_location(n);
  o << ">";
  /* We deliberately omit n.referent because this is a result of symbol
   * resolution and not morally a child of this node.
   */
  sync_to(n.loc.end);
  o << "</typeexprid>";
}

void XMLPrinter::visit_undefine(const Undefine &n) {
  sync_to(n);
  o << "<undefine ";
  add_location(n);
  o << ">";
  sync_to(*n.rhs);
  dispatch(*n.rhs);
  sync_to(n.loc.end);
  o << "</undefine>";
}

void XMLPrinter::visit_vardecl(const VarDecl &n) {
  sync_to(n);
  o << "<vardecl name=\"" << n.name << "\" readonly=\"" << n.readonly << "\" ";
  add_location(n);
  o << ">";
  sync_to(*n.type);
  o << "<type>";
  dispatch(*n.type);
  o << "</type>";
  sync_to(n.loc.end);
  o << "</vardecl>";
}

void XMLPrinter::visit_while(const While &n) {
  sync_to(n);
  o << "<while ";
  add_location(n);
  o << ">";
  sync_to(*n.condition);
  o << "<condition>";
  dispatch(*n.condition);
  o << "</condition>";
  if (!n.body.empty()) {
    sync_to(*n.body[0]);
    o << "<body>";
    for (auto &s : n.body) {
      sync_to(*s);
      dispatch(*s);
    }
    o << "</body>";
  }
  sync_to(n.loc.end);
  o << "</while>";
}

void XMLPrinter::visit_xor(const Xor &n) { visit_bexpr("xor", n); }

XMLPrinter::~XMLPrinter() {
  sync_to();
  o << "</unit>\n";
  o.flush();
}

void XMLPrinter::sync_to(const Node &n) { sync_to(n.loc.begin); }

void XMLPrinter::sync_to(const position &pos) {

  // the type of position.line and position.column changes across Bison
  // releases, so avoid some -Wsign-compare warnings by casting them in advance
  auto pos_line = static_cast<unsigned long>(pos.line);
  auto pos_col = static_cast<unsigned long>(pos.column);

  while (in.good() &&
         (line < pos_line || (line == pos_line && column < pos_col))) {

    int c = in.get();
    if (c == EOF) {
      break;
    }

    o << xml_escape(static_cast<char>(c));

    if (c == '\n') {
      line++;
      column = 1;
    } else {
      column++;
    }
  }
}

void XMLPrinter::add_location(const Node &n) {
  o << "first_line=\"" << n.loc.begin.line << "\" first_column=\""
    << n.loc.begin.column << "\" last_line=\"" << n.loc.end.line
    << "\" last_column=\"" << n.loc.end.column << "\"";
}

void XMLPrinter::visit_bexpr(const std::string &tag, const BinaryExpr &n) {
  sync_to(n);
  o << "<" << tag << " ";
  add_location(n);
  o << ">";
  sync_to(*n.lhs);
  o << "<lhs>";
  dispatch(*n.lhs);
  o << "</lhs>";
  sync_to(*n.rhs);
  o << "<rhs>";
  dispatch(*n.rhs);
  o << "</rhs>";
  sync_to(n.loc.end);
  o << "</" << tag << ">";
}

void XMLPrinter::visit_uexpr(const std::string &tag, const UnaryExpr &n) {
  sync_to(n);
  o << "<" << tag << " ";
  add_location(n);
  o << ">";
  sync_to(*n.rhs);
  o << "<rhs>";
  dispatch(*n.rhs);
  o << "</rhs>";
  sync_to(n.loc.end);
  o << "</" << tag << ">";
}
