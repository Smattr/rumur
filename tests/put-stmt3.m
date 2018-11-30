-- Test put statements get parsed correctly.

var
  x: boolean;

startstate begin
  put x;
  x := false;
end;

rule begin
  put x;
  x := !x;
end;
