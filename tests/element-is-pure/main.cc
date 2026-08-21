/// @file
/// @brief Test how array indexing purity is computed
///
/// It was discovered that array indexing was always considered a side-effect
/// free operation, even when its components had side effects. This tester
/// validates that this bug has not been reintroduced.

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <rumur/rumur.h>
#include <sstream>

using namespace rumur;

int main(void) {

  // a model that contains an array index with an impure expression
  const char src[] = "\
  var\
    x: boolean;\
    y: boolean;\
  \
  function foo(): 0..1 begin\
    y := !y;\
    return 0;\
  end;\
  \
  startstate begin\
    x := false;\
    y := false;\
  end;\
  \
  rule\
    var z: array[0..1] of boolean;\
  begin\
    z[0] := !x;\
    x := z[foo()];\
  end;\
  ";

  // parse this into an AST
  std::istringstream ss{src};
  Ptr<Model> m = parse_model(ss);

  // resolve symbols and sanity check
  resolve_symbols(*m);
  validate(*m);

  // locate the expression `z[foo()]`
  auto r = dynamic_cast<const SimpleRule *>(m->children[4].get());
  assert(r != nullptr);
  auto s = dynamic_cast<const Assignment *>(r->body[1].get());
  assert(s != nullptr);
  auto e = dynamic_cast<const Element *>(s->rhs.get());
  assert(e != nullptr);

  // it has side effects (within foo()), so should not be considered pure
  if (e->is_pure()) {
    std::cerr << "calling an impure function within an array index is "
                 "incorrectly considered pure\n";
    return EXIT_FAILURE;
  }

  return 0;
}
