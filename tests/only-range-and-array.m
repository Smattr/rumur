-- rumur_flags: ['--deadlock-detection', 'stuck']

type
  range_t: 0 .. 1;
  array_t: array[range_t] of range_t;

var
  x: array_t;

startstate begin
   x[0] := 0;
   x[1] := 0;
end;

rule begin
  x[0] := 1 - x[1];
  x[1] := 1 - x[0];
end;
