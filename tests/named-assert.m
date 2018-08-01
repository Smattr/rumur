-- A based assertion with a name given to it

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  assert x | !x "hello world";
  x := !x;
end;
