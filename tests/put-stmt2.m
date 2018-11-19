-- Test put statements get parsed correctly.

var
  x: 0 .. 10;

startstate begin
  x := 0;
end;

rule x > 0 ==> begin
  put "in first rule, x is ";
  put x;
  put "\n";
  x := x - 1;
end;

rule x < 10 ==> begin
  put "in second rule, x is ";
  put x;
  put "\n";
  x := x + 1;
end;
