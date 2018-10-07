-- rumur_exit_code: 1
-- Test using a non-boolean value in a conditional, which should be rejected.

var
  x: 0 .. 10;

startstate begin
  x := 1;
end;

rule begin
  if x then
    x := 3 - x;
  end;
end;
