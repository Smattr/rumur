-- test comparison between array types

type
  t: array[0 .. 1] of 0 .. 1;

var
  x: t;
  y: t;

startstate begin
  x[0] := 0;
  x[1] := 1;
  y[0] := 0;
  y[1] := 1;
end;

rule begin
  -- test comparison in an if statement
  if x = y then
  end;

  -- test negative comparison
  if x != y then
  end;

  -- test in an assertion
  assert x = y;

  x[0] := 1 - x[0];
  y[0] := 1 - y[0];
end;
