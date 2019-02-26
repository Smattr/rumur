-- checker_exit_code: 1

-- Test writing a value out of range.

var
  x: 0..1;

startstate begin
  x := 0;
end;

rule begin
  x := x + 1;
end;
