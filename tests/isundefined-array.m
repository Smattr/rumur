-- rumur_exit_code: 1
-- Test of isundefined on arrays, that should not work.

var
  x: array [0 .. 0] of boolean;

startstate begin
  x[0] := true;
end;

rule begin
  x[0] := isundefined(x);
end;
