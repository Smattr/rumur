-- Test "boolean" in different cases

var
  x: boolean;
  y: Boolean;
  z: BOOLEAN;
  w: BoOlEaN;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;
