#include <cstddef>
#include "except.h"
#include <gmpxx.h>
#include "logic.h"
#include <string>

namespace smt {

// configuration for any logic supporting bitvectors
class BV : public Logic {

 private:
  static const size_t BITVECTOR_WIDTH;

 public:
  std::string integer_type(void) const final {
    return "(_ BitVec " + std::to_string(BITVECTOR_WIDTH) + ")";
  }

  std::string numeric_literal(const mpz_class &value) const final {
    return "(_ bv" + value.get_str() + " " + std::to_string(BITVECTOR_WIDTH)
      + ")";
  }

  std::string add(void) const final { return "bvadd";  }
  std::string div(void) const final { return "bvdiv";  }
  std::string geq(void) const final { return "bvsge";  }
  std::string gt(void) const final  { return "bvsgt";  }
  std::string leq(void) const final { return "bvsle";  }
  std::string lt(void) const final  { return "bvslt";  }
  std::string mod(void) const final { return "bvsmod"; }
  std::string mul(void) const final { return "bvmul";  }
  std::string neg(void) const final { return "bvneg";  }
  std::string sub(void) const final { return "bvsub";  }

};

const size_t BV::BITVECTOR_WIDTH = 64;

static const BV BV;

// configuration for any logic supporting integer arithmetic
class IA : public Logic {

 public:
  std::string integer_type(void) const final {
    return "Int";
  }

  std::string numeric_literal(const mpz_class &value) const final {
    return value.get_str();
  }

  std::string add(void) const final { return "+";   }
  std::string div(void) const final { return "div"; }
  std::string geq(void) const final { return ">=";  }
  std::string gt(void) const final  { return ">";   }
  std::string leq(void) const final { return "<=";  }
  std::string lt(void) const final  { return "<";   }
  std::string mod(void) const final { return "mod"; }
  std::string mul(void) const final { return "*";   }
  std::string neg(void) const final { return "-";   }
  std::string sub(void) const final { return "-";   }
};

static const IA IA;

const Logic &get_logic(const std::string &name) {

  // prefer integer reasoning
  if (name.find("IA") != std::string::npos)
    return IA;

  if (name.find("BV") != std::string::npos)
    return BV;

  throw Unsupported("unknown logic " + name);
}

}
