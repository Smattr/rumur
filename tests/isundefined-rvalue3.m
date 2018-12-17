-- rumur_exit_code: 1
-- Test of using a function rvalue in isundefined, which should be rejected

var
  x: boolean;

function foo(y: boolean): boolean; begin
  return isundefined(y);
end;

startstate begin
  x := true;
end;

rule foo(x) ==> begin
  x := !x;
end;
