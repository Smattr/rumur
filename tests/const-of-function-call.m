-- rumur_exit_code: 1

-- test that a constant whose value is a function call is rejected

var
  x: boolean;

function foo(): boolean; begin
  return true;
end;

startstate begin
  x := true;
end;

rule
  const N: foo();
begin
  x := !x;
end;
