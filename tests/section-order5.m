-- similar to section-order4.m, but the function references a state variable

var
  x: boolean;

startstate begin
  x := true;
end;

function foo(): boolean; begin
  return x;
end;

rule begin
  x := !foo();
end;
