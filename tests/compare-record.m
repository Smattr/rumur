-- test comparison between record types

type
  t: record
    x: boolean;
  end;

var
  x: t;
  y: t;

startstate begin
  x.x := true;
  y.x := true;
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

  x.x := !x.x;
  y.x := !y.x;
end;
