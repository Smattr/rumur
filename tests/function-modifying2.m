-- a variant of function-modifying.m that modifies x twice in one statement

var
  x: boolean;

function foo(var y: boolean): boolean; begin
  return !y;
end;

startstate begin
  x := true;
end;

rule begin
  x := foo(x);
end;
