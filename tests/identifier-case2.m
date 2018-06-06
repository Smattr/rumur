-- This test is designed to provoke any issues with handling the
-- case-sensitivity of variables.

var
  x: boolean;

startstate begin
  x := true;
end;

ruleset X: 0 .. 2 do
  rule begin
    x := X = 0;
  end;
end;
