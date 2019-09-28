-- Test that a function parameter does not prevent referencing a type of the
-- same name in the return type. This is pretty strange code to write, but it
-- falls into that "you know what I meant" category. Also, it is possible for a
-- code generator to inadvertently produce code like this.

type
  t: 0 .. 1;

var
  x: boolean;

function foo(t: boolean): t; begin
  return 0;
end;

startstate begin
  x := true;
end;

rule begin
  if foo(x) = 0 then
    x := !x;
  else
    x := !x;
  end;
end;
