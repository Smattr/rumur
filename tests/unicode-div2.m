-- test of unicode division symbol, solidus

var
  x: 1 .. 2;

startstate begin
  x := 1;
end;

rule begin
  x := 2 ∕ x;
end;
