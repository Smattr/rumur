/// @file
/// @brief Test how array indexing purity is computed
///
/// It was discovered that array indexing was always considered a side-effect
/// free operation, even when its components had side effects. This tester
/// validates that this bug has not been reintroduced.

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <rumur/rumur.h>
#include <sstream>

using namespace rumur;

int main(int argc, char **argv) {

  // a model that contains an array index with an impure expression
  const char src1[] = "\
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

  // a similar model but where the array is the impure expression
  const char src2[] = "\
  type\
    t1: record\
      a: array[0..1] of boolean;\
    end;\
    t2: record\
      b: array[0..1] of t1;\
    end;\
  \
  var\
    x: boolean;\
    y: boolean;\
  \
  function foo(): 0..1;\
  begin\
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
    var t: t2;\
  begin\
    t.b[0].a[0] := !x;\
    x := t.b[foo()].a[0];\
  end;\
  ";

  const char *src;
  assert(argc == 2);
  (void)argc;
  if (strcmp(argv[1], "index") == 0) {
    src = src1;
  } else if (strcmp(argv[1], "array") == 0) {
    src = src2;
  } else {
    std::cerr << "unrecognised command line argument\n";
    return EXIT_FAILURE;
  }

  // parse this into an AST
  std::istringstream ss{src};
  Ptr<Model> m = parse_model(ss);

  // resolve symbols and sanity check
  resolve_symbols(*m);
  validate(*m);

  // locate the expression `z[foo()]`
  auto r = dynamic_cast<const SimpleRule *>(m->children.back().get());
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
