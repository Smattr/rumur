-- rumur_exit_code: 1

-- test that defining a function after rules is rejected

var
  x: boolean;

startstate begin
  x := true;
end;

function foo(): boolean; begin
  return true;
end;

rule begin
  x := !x;
end;
