#include "Stage.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <rumur/rumur.h>
#include <string>

using namespace rumur;

Stage &Stage::operator<<(const std::string &s) {

  // We expect the input argument to contain a sequence of UTF-8 characters that
  // we want to pass to process() character-by-character. However iterating
  // through a std::string yields 8-bit chars, not UTF-8 characters. So we
  // commit a minor sin and roll our own UTF-8 character-by-character iteration.

  for (size_t i = 0; i < s.size();) {

    // clamp the size of the current character to at most this to avoid
    // overrunning the string even in the case of malformed UTF-8
    size_t length = s.size() - i;

    // From the UTF-8 RFC (3629):
    //
    //   Char. number range  |        UTF-8 octet sequence
    //      (hexadecimal)    |              (binary)
    //   --------------------+---------------------------------------------
    //   0000 0000-0000 007F | 0xxxxxxx
    //   0000 0080-0000 07FF | 110xxxxx 10xxxxxx
    //   0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
    //   0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    //
    // So we can determine the length of the current character by the bits set
    // in the first byte.

    uint8_t leader = static_cast<uint8_t>(s[i]);

    if ((leader >> 7) == 0) {
      length = length < 1 ? length : 1;

    } else if ((leader >> 5) == 6) {
      length = length < 2 ? length : 2;

    } else if ((leader >> 4) == 14) {
      length = length < 3 ? length : 3;

    } else if ((leader >> 3) == 30) {
      length = length < 4 ? length : 4;

    } else {
      // Malformed. Just treat it as a 1-byte character.
      length = 1;
    }

    // write out this character
    Token t(s.substr(i, length));
    process(t);

    // move to the next character
    assert(i + length <= s.size());
    i += length;
  }

  return *this;
}

void Stage::attach(Stage &top_) { top = &top_; }

IntermediateStage::IntermediateStage(Stage &next_) : next(next_) {}

// by default, pass through to the next stage
void IntermediateStage::visit_add(const Add &n) { next.visit_add(n); }
void IntermediateStage::visit_aliasdecl(const AliasDecl &n) {
  next.visit_aliasdecl(n);
}
void IntermediateStage::visit_aliasrule(const AliasRule &n) {
  next.visit_aliasrule(n);
}
void IntermediateStage::visit_aliasstmt(const AliasStmt &n) {
  next.visit_aliasstmt(n);
}
void IntermediateStage::visit_and(const And &n) { next.visit_and(n); }
void IntermediateStage::visit_array(const Array &n) { next.visit_array(n); }
void IntermediateStage::visit_assignment(const Assignment &n) {
  next.visit_assignment(n);
}
void IntermediateStage::visit_band(const Band &n) { next.visit_band(n); }
void IntermediateStage::visit_bnot(const Bnot &n) { next.visit_bnot(n); }
void IntermediateStage::visit_bor(const Bor &n) { next.visit_bor(n); }
void IntermediateStage::visit_clear(const Clear &n) { next.visit_clear(n); }
void IntermediateStage::visit_constdecl(const ConstDecl &n) {
  next.visit_constdecl(n);
}
void IntermediateStage::visit_div(const Div &n) { next.visit_div(n); }
void IntermediateStage::visit_element(const Element &n) {
  next.visit_element(n);
}
void IntermediateStage::visit_enum(const Enum &n) { next.visit_enum(n); }
void IntermediateStage::visit_eq(const Eq &n) { next.visit_eq(n); }
void IntermediateStage::visit_errorstmt(const ErrorStmt &n) {
  next.visit_errorstmt(n);
}
void IntermediateStage::visit_exists(const Exists &n) { next.visit_exists(n); }
void IntermediateStage::visit_exprid(const ExprID &n) { next.visit_exprid(n); }
void IntermediateStage::visit_field(const Field &n) { next.visit_field(n); }
void IntermediateStage::visit_for(const For &n) { next.visit_for(n); }
void IntermediateStage::visit_forall(const Forall &n) { next.visit_forall(n); }
void IntermediateStage::visit_function(const Function &n) {
  next.visit_function(n);
}
void IntermediateStage::visit_functioncall(const FunctionCall &n) {
  next.visit_functioncall(n);
}
void IntermediateStage::visit_geq(const Geq &n) { next.visit_geq(n); }
void IntermediateStage::visit_gt(const Gt &n) { next.visit_gt(n); }
void IntermediateStage::visit_if(const If &n) { next.visit_if(n); }
void IntermediateStage::visit_ifclause(const IfClause &n) {
  next.visit_ifclause(n);
}
void IntermediateStage::visit_implication(const Implication &n) {
  next.visit_implication(n);
}
void IntermediateStage::visit_isundefined(const IsUndefined &n) {
  next.visit_isundefined(n);
}
void IntermediateStage::visit_leq(const Leq &n) { next.visit_leq(n); }
void IntermediateStage::visit_lsh(const Lsh &n) { next.visit_lsh(n); }
void IntermediateStage::visit_lt(const Lt &n) { next.visit_lt(n); }
void IntermediateStage::visit_mod(const Mod &n) { next.visit_mod(n); }
void IntermediateStage::visit_model(const Model &n) { next.visit_model(n); }
void IntermediateStage::visit_mul(const Mul &n) { next.visit_mul(n); }
void IntermediateStage::visit_negative(const Negative &n) {
  next.visit_negative(n);
}
void IntermediateStage::visit_neq(const Neq &n) { next.visit_neq(n); }
void IntermediateStage::visit_not(const Not &n) { next.visit_not(n); }
void IntermediateStage::visit_number(const Number &n) { next.visit_number(n); }
void IntermediateStage::visit_or(const Or &n) { next.visit_or(n); }
void IntermediateStage::visit_procedurecall(const ProcedureCall &n) {
  next.visit_procedurecall(n);
}
void IntermediateStage::visit_property(const Property &n) {
  next.visit_property(n);
}
void IntermediateStage::visit_propertyrule(const PropertyRule &n) {
  next.visit_propertyrule(n);
}
void IntermediateStage::visit_propertystmt(const PropertyStmt &n) {
  next.visit_propertystmt(n);
}
void IntermediateStage::visit_put(const Put &n) { next.visit_put(n); }
void IntermediateStage::visit_quantifier(const Quantifier &n) {
  next.visit_quantifier(n);
}
void IntermediateStage::visit_range(const Range &n) { next.visit_range(n); }
void IntermediateStage::visit_record(const Record &n) { next.visit_record(n); }
void IntermediateStage::visit_return(const Return &n) { next.visit_return(n); }
void IntermediateStage::visit_rsh(const Rsh &n) { next.visit_rsh(n); }
void IntermediateStage::visit_ruleset(const Ruleset &n) {
  next.visit_ruleset(n);
}
void IntermediateStage::visit_scalarset(const Scalarset &n) {
  next.visit_scalarset(n);
}
void IntermediateStage::visit_simplerule(const SimpleRule &n) {
  next.visit_simplerule(n);
}
void IntermediateStage::visit_startstate(const StartState &n) {
  next.visit_startstate(n);
}
void IntermediateStage::visit_sub(const Sub &n) { next.visit_sub(n); }
void IntermediateStage::visit_switch(const Switch &n) { next.visit_switch(n); }
void IntermediateStage::visit_switchcase(const SwitchCase &n) {
  next.visit_switchcase(n);
}
void IntermediateStage::visit_ternary(const Ternary &n) {
  next.visit_ternary(n);
}
void IntermediateStage::visit_typedecl(const TypeDecl &n) {
  next.visit_typedecl(n);
}
void IntermediateStage::visit_typeexprid(const TypeExprID &n) {
  next.visit_typeexprid(n);
}
void IntermediateStage::visit_undefine(const Undefine &n) {
  next.visit_undefine(n);
}
void IntermediateStage::visit_vardecl(const VarDecl &n) {
  next.visit_vardecl(n);
}
void IntermediateStage::visit_while(const While &n) { next.visit_while(n); }
void IntermediateStage::visit_xor(const Xor &n) { next.visit_xor(n); }

void IntermediateStage::process(const Token &t) { next.process(t); }

void IntermediateStage::sync_to(const Node &n) { next.sync_to(n); }

void IntermediateStage::sync_to(const position &pos) { next.sync_to(pos); }

void IntermediateStage::skip_to(const Node &n) { next.skip_to(n); }

void IntermediateStage::skip_to(const position &pos) { next.skip_to(pos); }

IntermediateStage::~IntermediateStage() {}
