-- The following tests a case where we return a variable of a range type that
-- differs from the function's return type. This does not exceed the type bounds
-- and should be accepted -- it is accepted by CMurphi -- but Rumur at commit
-- 84998945e6f0f2f3d15b2c9d3bb50e953ffb5143 rejects it.

type
  t1: 0 .. 10;
  t2: 0 .. 11;

var
  x: t1;

function foo(): t2; begin
  return x;
end;

startstate begin
  x := 0;
end;

rule begin
  x := 10 - foo();
end;
