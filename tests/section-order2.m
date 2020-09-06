-- test that defining a function after rules (Rumur extension) is accepted

var
  x: boolean;

startstate begin
  x := true;
end;

function foo(): boolean; begin
  return true;
end;

rule begin
  x := !x;
end;
