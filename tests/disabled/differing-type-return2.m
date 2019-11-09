-- checker_exit_code: 1

-- A variant of differing-type-return.m that should be accepted for code
-- generation but then should fail at runtime.

-- FIXME: *Should* this actually fail at runtime? The generated code currently
-- results in a function that returns an rvalue (value_t), so everything works
-- out fine. If the user had actually written this, it would be clear what they
-- meant. I'm inclined to think maybe this should just be accepted...?

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
