-- This test is designed to provoke any issues with handling the
-- case-sensitivity of variables.

var
  x: boolean;

startstate begin
  x := true;
end;

ruleset X: boolean do
  rule begin
    x := !X;
  end;
end;
