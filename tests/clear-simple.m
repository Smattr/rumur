-- Test of the clear statement on a simple type

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  clear x;
end;

rule begin
  x := true;
end;
