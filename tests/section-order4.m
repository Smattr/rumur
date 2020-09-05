-- rumur_exit_code: 1

-- similar to section-order2.m, but the function is used

var
  x: boolean;

startstate begin
  x := true;
end;

function foo(): boolean; begin
  return true;
end;

rule begin
  x := foo() & !x;
end;
