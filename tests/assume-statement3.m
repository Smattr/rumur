-- Similar to assume-statement2.m, but with the assumption inside a function.

var
  x: 0 .. 2

procedure foo(a: 0 .. 2); begin
  assume a != 2;
end;

startstate begin
  x := 0;
end

rule x > 0 ==> begin
  x := x - 1;
end

rule x < 2 ==> begin
  x := x + 1;
  foo(x);
end

-- if the assumption is working correctly, this invariant should pass
invariant x != 2
