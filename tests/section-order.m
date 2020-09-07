-- test that defining a function prior to variables/constants/types (Rumur
-- extension) is accepted

function foo(): boolean; begin
  return true;
end;

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;
