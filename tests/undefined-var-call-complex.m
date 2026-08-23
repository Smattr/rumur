-- a model that passes `undefined` to a function taking complex type

type
  t: record
    a: 0..1;
  end;

var
  x: boolean;

function foo(var y: t): boolean; begin
  assert isundefined(y.a);
  return !x;
end;

startstate begin
  x := false;
end;

rule begin
  x := foo(undefined);
end;
