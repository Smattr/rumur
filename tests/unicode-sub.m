-- test of unicode subtraction symbol

var
  x: 0 .. 1;

startstate begin
  x := 0;
end;

rule begin
  x := 1 − x;
end;
