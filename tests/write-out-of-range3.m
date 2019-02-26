-- checker_exit_code: 1

-- Test writing a value out of range into an array.

var
  x: array[0..1] of 0..1;

startstate begin
  x[0] := 0;
end;

rule begin
  x[0] := x[0] + 1;
end;
