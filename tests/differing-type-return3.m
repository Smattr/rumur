-- A variant of differing-type-return.m where we use the same type for both
-- entities but indirect through a type definition.

type
  t1: 0 .. 10;
  t2: t1;

var
  x: t2;

function foo(): t1; begin
  return x;
end;

startstate begin
  x := 0;
end;

rule begin
  x := 10 - foo();
end;
