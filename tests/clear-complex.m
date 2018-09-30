-- Test of the clear statement on a complex type

var
  x: record a: boolean end;

startstate begin
  x.a := true;
end;

rule begin
  clear x;
end;

rule begin
  x.a := true;
end;
