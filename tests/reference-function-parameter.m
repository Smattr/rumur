var
  x: boolean;

function foo(y: boolean): boolean; begin
  return y;
end;

startstate begin
  x := true;
end;

rule begin
  x := !foo(x);
end;
