-- rumur_exit_code: 1

-- test that defining a function prior to variables/constants/types is rejected

function foo(): boolean; begin
  return true;
end;

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;
