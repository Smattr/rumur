var
  x: boolean;

function foo(a: boolean; b: boolean): boolean; begin
  return a;
end;

startstate begin
  x := true;
end;

rule begin
  x := !foo(x, x);
end;
