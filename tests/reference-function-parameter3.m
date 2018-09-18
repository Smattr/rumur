var
  x: boolean;

function foo(var y: boolean): boolean; begin
  y := !y;
  return y;
end;

startstate begin
  x := true;
end;

rule begin
  foo(x);
end;
