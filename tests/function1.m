-- Preliminary test of complex-returning function implementation
-- Yes, the model below makes no sense.

type
 t: record
   x: boolean;
 end;

var
  y: t;

function foo(): t; begin
  return y;
end;

startstate begin
  y.x := true;
end;

rule begin
  y := foo();
  y.x := !y.x;
end;
