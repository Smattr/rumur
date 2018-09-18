var
  x: boolean;

function foo(var y: boolean): boolean; begin
  return y;
end;

startstate begin
  x := true;
end;

rule begin
  x := !foo(x);
end;
