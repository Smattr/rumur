-- a variant of alias-and-field.m that demonstrates a similar problem, but using
-- a non-read-only function parameter that collides with a field name

type
  t: record
    x: boolean;
  end;

var
  y: t;
  z: boolean;

procedure foo(var x: boolean); begin
  y.x := !x;
end;

startstate begin
  y.x := true;
end;

rule begin
  foo(y.x);
end;
