-- checker_exit_code: 1

-- A variant of differing-type-return.m that should be accepted for code
-- generation but then should fail at runtime.

type
  t1: 0 .. 10;
  t2: 0 .. 11;

var
  x: t2;

function foo(): t1; begin
  return x;
end;

startstate begin
  x := 0;
end;

rule begin
  x := 11 - foo();
end;
