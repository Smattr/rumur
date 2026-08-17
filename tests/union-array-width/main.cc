/// @file
/// @brief Test how union width is computed
///
/// After implementing union types, it was discovered that their widths were
/// miscalculated in some circumstances. This tester validates that this bug has
/// not been reintroduced.

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <gmpxx.h>
#include <iostream>
#include <rumur/rumur.h>
#include <sstream>

using namespace rumur;

int main(void) {

  // A model that contains a union of an array type. The key feature here is
  // that the element type of the array has a count that is not a power of 2.
  const char src[] = "\
  type\
    a_t: array[0..1] of 0..2;\
    u_t: union { a_t };\
  \
  var\
    x: u_t;\
  \
  startstate begin\
  end;\
  ";

  // parse this into an AST
  std::istringstream ss{src};
  Ptr<Model> m = parse_model(ss);

  // resolve symbols and sanity check
  resolve_symbols(*m);
  validate(*m);

  // what is the width of the array type?
  auto array_def = dynamic_cast<const TypeDecl *>(m->children[0].get());
  assert(array_def != nullptr);
  const mpz_class w1 = array_def->value->width();
  std::cout << "`a_t: array[0..1] of 0..2` width is " << w1.get_str() << '\n';

  // what is the width of the union type?
  auto union_def = dynamic_cast<const TypeDecl *>(m->children[1].get());
  assert(union_def != nullptr);
  const mpz_class w2 = union_def->value->width();
  std::cout << "`u_t: union { a_t }` width is " << w2.get_str() << '\n';

  // if union width calculation was done correctly, these should match
  if (w1 != w2) {
    std::cerr
        << "union of a single type has a different width than its member\n";
    return EXIT_FAILURE;
  }

  return 0;
}
