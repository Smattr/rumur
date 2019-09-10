-- a variant on recursion4.m where the recursive function is used as a guard

var
  x: boolean;

function foo(x: boolean): boolean; begin
  return !x;
end;

function bar(x: boolean): boolean; begin
  foo(x);
  if x then
    bar(!x);
  end;
  return !x;
end;

startstate begin
  x := true;
end;

rule begin
  x := bar(x);
end;

rule bar(x) ==> begin
  x := !x;
end;
