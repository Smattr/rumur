-- test that shadowing a type with a function parameter works as expected

type
  t: 0 .. 1;

var
  x: boolean;

function foo(t: boolean): boolean; begin
  return !t;
end;

startstate begin
  x := true;
end;

rule begin
  x := foo(x);
end;
