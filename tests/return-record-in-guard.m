-- A function returning a record that is then used within a guard. This
-- previously resulted in malformed code being emitted.
-- https://github.com/Smattr/rumur/issues/290

type
  x_t: record
    y : boolean;
  end;

var
  z: boolean;

function produce(): x_t; 
var x: x_t;
begin
  x.y := true;
  return x;
end;

function consume(x: x_t): boolean;
begin
  return x.y;
end;

startstate begin
  z := true;
end;

rule consume(produce()) ==> begin
  z := !z;
end;
