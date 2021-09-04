#include "generate_h.h"
#include "CLikeGenerator.h"
#include "options.h"
#include "resources.h"
#include <cstddef>
#include <iostream>
#include <rumur/rumur.h>
#include <string>
#include <vector>

using namespace rumur;

namespace {

class HGenerator : public CLikeGenerator {

public:
  HGenerator(const std::vector<Comment> &comments_, std::ostream &out_,
             bool pack_)
      : CLikeGenerator(comments_, out_, pack_) {}

  void visit_constdecl(const ConstDecl &n) final {
    *this << indentation() << "extern const ";

    // replicate the logic from CGenerator::visit_constdecl
    if (n.type != nullptr) {
      *this << *n.type;
    } else {
      const Ptr<TypeExpr> type = n.value->type();
      auto it = enum_typedefs.find(type->unique_id);
      if (it != enum_typedefs.end()) {
        *this << it->second;
      } else {
        *this << "__typeof__(" << *n.value << ")";
      }
    }

    *this << " " << n.name << ";";
    emit_trailing_comments(n);
    *this << "\n";
  }

  void visit_function(const Function &n) final {
    *this << indentation();
    if (n.return_type == nullptr) {
      *this << "void";
    } else {
      *this << *n.return_type;
    }
    *this << " " << n.name << "(";
    if (n.parameters.empty()) {
      *this << "void";
    } else {
      std::string sep;
      for (const Ptr<VarDecl> &p : n.parameters) {
        *this << sep << *p->type << " ";
        // if this is a var parameter, it needs to be a pointer
        if (!p->readonly) {
          (void)is_pointer.insert(p->unique_id);
          *this << "*";
        }
        *this << p->name;
        sep = ", ";
      }
    }
    *this << ");\n";

    // discard any comments related to declarations and statements within this
    // function
    drop_comments(n.loc.end);
  }

  void visit_propertyrule(const PropertyRule &n) final {

    // function prototype
    *this << indentation() << "bool " << n.name << "(";

    // parameters
    if (n.quantifiers.empty()) {
      *this << "void";
    } else {
      std::string sep;
      for (const Quantifier &q : n.quantifiers) {
        *this << sep;
        if (auto t = dynamic_cast<const TypeExprID *>(q.type.get())) {
          *this << t->name;
        } else {
          *this << value_type;
        }
        *this << " " << q.name;
        sep = ", ";
      }
    }

    *this << ");\n";
  }

  void visit_simplerule(const SimpleRule &n) final {
    *this << indentation() << "bool guard_" << n.name << "(";

    // parameters
    if (n.quantifiers.empty()) {
      *this << "void";
    } else {
      std::string sep;
      for (const Quantifier &q : n.quantifiers) {
        *this << sep;
        if (auto t = dynamic_cast<const TypeExprID *>(q.type.get())) {
          *this << t->name;
        } else {
          *this << value_type;
        }
        *this << " " << q.name;
        sep = ", ";
      }
    }

    *this << ");\n\n";

    *this << indentation() << "void rule_" << n.name << "(";

    // parameters
    if (n.quantifiers.empty()) {
      *this << "void";
    } else {
      std::string sep;
      for (const Quantifier &q : n.quantifiers) {
        *this << sep;
        if (auto t = dynamic_cast<const TypeExprID *>(q.type.get())) {
          *this << t->name;
        } else {
          *this << value_type;
        }
        *this << " " << q.name;
        sep = ", ";
      }
    }

    *this << ");\n";

    // discard any comments associated with things within this rule
    drop_comments(n.loc.end);
  }

  void visit_startstate(const StartState &n) final {
    *this << indentation() << "void startstate_" << n.name << "(";

    // parameters
    if (n.quantifiers.empty()) {
      *this << "void";
    } else {
      std::string sep;
      for (const Quantifier &q : n.quantifiers) {
        *this << sep;
        if (auto t = dynamic_cast<const TypeExprID *>(q.type.get())) {
          *this << t->name;
        } else {
          *this << value_type;
        }
        *this << " " << q.name;
        sep = ", ";
      }
    }

    *this << ");\n";

    // discard any comments associated with things within this rule
    drop_comments(n.loc.end);
  }

  void visit_vardecl(const VarDecl &n) final {
    *this << indentation();
    if (n.is_in_state())
      *this << "extern ";
    *this << *n.type << " " << n.name << ";";
    emit_trailing_comments(n);
    *this << "\n";
  }
};

} // namespace

void generate_h(const Node &n, const std::vector<Comment> &comments, bool pack,
                std::ostream &out) {

  // write the static prefix to the beginning of the source file
  for (size_t i = 0; i < resources_h_prefix_h_len; i++)
    out << (char)resources_h_prefix_h[i];

  HGenerator gen(comments, out, pack);
  gen.dispatch(n);

  // close the `extern "C"` block opened in ../resources/h_prefix.h
  out << "\n"
      << "#ifdef __cplusplus\n"
      << "}\n"
      << "#endif\n";
}
