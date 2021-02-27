#include <cstddef>
#include <rumur/Property.h>
#include <rumur/Ptr.h>
#include <rumur/traverse.h>

namespace rumur {

Property::Property(Category category_, const Ptr<Expr> &expr_,
                   const location &loc_)
    : Node(loc_), category(category_), expr(expr_) {}

Property *Property::clone() const { return new Property(*this); }

void Property::visit(BaseTraversal &visitor) { visitor.visit_property(*this); }

void Property::visit(ConstBaseTraversal &visitor) const {
  visitor.visit_property(*this);
}

} // namespace rumur
