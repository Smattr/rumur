-- checker_exit_code: 1

-- Test reading of an undefined array value.

var
  x: array[0 .. 1] of boolean;

startstate begin
end;

rule begin
  x[0] := !x[0];
end;
