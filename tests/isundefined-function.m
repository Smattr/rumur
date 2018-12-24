-- Test of using isundefined in various function contexts.

type
  t: record
    z: boolean;
  end;

var
  x: boolean;
  y: t;

procedure foo(var a: boolean); begin
  if isundefined(a) then
  end;
end;

procedure bar(a: boolean); begin
  if isundefined(a) then
  end;
end;

function baz(var a: boolean): boolean; begin
  return isundefined(a);
end;

function qux(a: boolean): boolean; begin
  return isundefined(a);
end;

function quux(a: t): boolean; begin
  return isundefined(a.z);
end;

function quuz(var a: t): boolean; begin
  return isundefined(a.z);
end;

startstate begin
  x := true;
end;

rule begin
  foo(x);
  bar(x);
  baz(x);
  qux(x);

  foo(y.z);
  bar(y.z);
  baz(y.z);
  qux(y.z);

  quux(y);
  quuz(y);

  x := !x;
end;
