-- checker_exit_code: 1

-- test octal literals are interpreted correctly

var
  x: 0 .. 010;

startstate begin
  x := 0;
end;

rule begin
  -- if the upper bound of the type was correctly parsed as an octal, it should
  -- have been set to 8 and we should trigger an error here due to assigning a
  -- value above this bound
  x := 9 - x;
end;
