-- rumur_flags: ['--deadlock-detection', 'stuck']

type
  foo_t: array[0 .. 1] of boolean;

var
  x: foo_t;

startstate begin
  x[0] := true;
  x[1] := false;
end;

rule begin
  x[0] := x[1];
  x[1] := x[0];
end;
