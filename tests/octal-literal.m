-- rumur_exit_code: 1

-- test that invalid octal literals are rejected

var
  x: 0 .. 09;

startstate begin
  x := 1;
end;

rule begin
  x := 1 - x;
end;
