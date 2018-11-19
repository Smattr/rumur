-- Test put statements get parsed correctly.

var
  x: boolean;

startstate begin
  x := false;
end;

rule begin
  put "x is ";
  put x;
  put "\n";
  x := !x;
end;
