#include <cstddef>
#include "except.h"
#include <gmpxx.h>
#include "logic.h"
#include <string>
#include <unordered_map>

namespace smt {

static const size_t BITVECTOR_WIDTH = 64;

Logic::Logic(bool bv, bool ia):
    bitvectors(bv),
    integers(ia) { }

std::string Logic::integer_type() const {

  if (integers)
    return "Int";

  if (bitvectors)
    return "(_ BitVec " + std::to_string(BITVECTOR_WIDTH) + ")";

  throw Unsupported();
}

std::string Logic::numeric_literal(const mpz_class &value) const {

  if (integers)
    return value.get_str();

  if (bitvectors)
    return "(_ bv" + value.get_str() + " " + std::to_string(BITVECTOR_WIDTH)
      + ")";

  throw Unsupported();
}

std::string Logic::add(void) const {

  if (integers)
    return "+";

  if (bitvectors)
    return "bvadd";

  throw Unsupported();
}

std::string Logic::div(void) const {

  if (integers) {
    /* XXX: may cause solvers like CVC4 to fail with an error. Not visible to
     * the user unless passing --debug though, so left as-is for now.
     */
    return "div";
  }

  if (bitvectors)
    return "bvsdiv";

  throw Unsupported();
}

std::string Logic::geq(void) const {

  if (integers)
    return ">=";

  if (bitvectors)
    return "bvsge";

  throw Unsupported();
}

std::string Logic::gt(void) const {
  
  if (integers)
    return ">";
    
  if (bitvectors)
    return "bvsgt";
    
  throw Unsupported();
}

std::string Logic::leq(void) const {
  
  if (integers)
    return "<=";
    
  if (bitvectors)
    return "bvsle";
    
  throw Unsupported();
}

std::string Logic::lt (void) const {
  
  if (integers)
    return "<";
    
  if (bitvectors)
    return "bvslt";
    
  throw Unsupported();
}

std::string Logic::mod(void) const {

  if (integers)
    return "mod";

  if (bitvectors)
    return "bvsmod";
    
  throw Unsupported();
}

std::string Logic::mul(void) const {
  
  if (integers)
    return "*";
    
  if (bitvectors)
    return "bvmul";
    
  throw Unsupported();
}

std::string Logic::neg(void) const {
  
  if (integers)
    return "-";
    
  if (bitvectors)
    return "bvneg";
    
  throw Unsupported();
}

std::string Logic::sub(void) const {
  
  if (integers)
    return "-";
    
  if (bitvectors)
    return "bvsub";
    
  throw Unsupported();
}

static const std::unordered_map<std::string, Logic> LOGICS = {
                 //    BV     IA
  { "ALIA",      Logic(false, true ) },
  { "AUFLIA",    Logic(false, true ) },
  { "AUFLIRA",   Logic(false, true ) },
  { "AUFNIRA",   Logic(false, true ) },
  { "LIA",       Logic(false, true ) },
  { "LRA",       Logic(false, false) },
  { "NIA",       Logic(false, true ) },
  { "NRA",       Logic(false, false) },
  { "QF_ABV",    Logic(true,  false) },
  { "QF_ALIA",   Logic(false, true ) },
  { "QF_AUFBV",  Logic(true,  false) },
  { "QF_AUFLIA", Logic(false, true ) },
  { "QF_AX",     Logic(false, false) },
  { "QF_BV",     Logic(true,  false) },
  { "QF_IDL",    Logic(false, false) },
  { "QF_LIA",    Logic(false, true ) },
  { "QF_LRA",    Logic(false, false) },
  { "QF_NIA",    Logic(false, true ) },
  { "QF_NRA",    Logic(false, false) },
  { "QF_RDL",    Logic(false, false) },
  { "QF_UF",     Logic(false, false) },
  { "QF_UFBV",   Logic(true,  false) },
  { "QF_UFIDL",  Logic(false, false) },
  { "QF_UFLIA",  Logic(false, true ) },
  { "QF_UFLRA",  Logic(false, false) },
  { "QF_UFNIA",  Logic(false, true ) },
  { "QF_UFNRA",  Logic(false, false) },
  { "UFLRA",     Logic(false, false) },
  { "UFNIA",     Logic(false, true ) },
};

const Logic &get_logic(const std::string &name) {

  auto it = LOGICS.find(name);
  if (it != LOGICS.end())
    return it->second;

  throw Unsupported("unknown logic " + name);
}

}
