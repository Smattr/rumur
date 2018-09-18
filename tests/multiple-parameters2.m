-- Same as multiple-parameters.m but omitting a semi-colon (Rumur extension).

var
  x: boolean;

function foo(a: boolean b: boolean): boolean; begin
  return a;
end;

startstate begin
  x := true;
end;

rule begin
  x := !foo(x, x);
end;
