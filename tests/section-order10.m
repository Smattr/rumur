-- The following tests an edge case where function parameters do *not* shadow
-- global symbols, but *do* shadow global symbols if sections are reordered.
-- I.e. if all variable and constant definitions were hoisted to the top of the
-- file. When implementing the patch series to lift the restriction on section
-- ordering, it was observed that the optimisation that reorders record fields
-- for faster access (--reorder-fields ...) caused variable definitions to be
-- hoisted in this way. This was previously no problem, but now may cause some
-- surprises like this and needs careful handling.

var x: boolean;

function foo(y: boolean): boolean; begin
  assert y = true | y = false;
  return y;
end;

const y: 5;

function bar(z: boolean): boolean; begin
  assert z = true | z = false;
  return z;
end;

var z: 0 .. 10;

startstate begin
  x := true;
  z := 8;
end;

rule begin
  x := !bar(foo(x));
end;
