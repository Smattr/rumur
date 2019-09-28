-- Test of a basic cover property

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;

cover "x was false" !x;
