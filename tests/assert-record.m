-- rumur_exit_code: 1

-- asserting something that is not boolean should be rejected

type
  bar: record x: array[0..1] of 0..1; end;

var
  baz: bar;

startstate begin
  baz.x[0] := 0;
end;

rule begin
  assert baz.x;
end;
