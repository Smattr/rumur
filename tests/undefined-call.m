-- a model that passes `undefined` to a function

var
  x: boolean;

function foo(y: 0..1): boolean; begin
  assert isundefined(y);
  return !x;
end;

function bar(a: boolean; y: 0..1): boolean; begin
  assert isundefined(y);
  return !a;
end;

function baz(y: 0..1; a: boolean): boolean; begin
  assert isundefined(y);
  return !a;
end;

function qux(a: boolean; y: 0..1; b: boolean): boolean; begin
  assert isundefined(y);
  assert a = b;
  return !a;
end;

startstate begin
  x := false;
end;

rule begin
  x := foo(undefined);
end;

rule begin
  x := bar(x, undefined);
end;

rule begin
  x := baz(undefined, x);
end;

rule begin
  x := qux(x, undefined, x);
end;
