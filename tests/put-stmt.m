-- Test put statements get parsed correctly.

var
  x: boolean;

startstate begin
  x := false;
end;

rule begin
  put "hello world";
  put "\n";
  x := !x;
end;
