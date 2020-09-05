-- rumur_exit_code: 1

-- similar to section-order2.m, but the function is called illegally

var
  x: boolean;

startstate begin
  x := foo();
end;

function foo(): boolean; begin
  return true;
end;

rule begin
  x := !x;
end;
