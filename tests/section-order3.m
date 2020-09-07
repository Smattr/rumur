-- test that defining a const after rules (Rumur extension) is allowed

var
  x: boolean;

startstate begin
  x := true;
end;

const N: 3;

rule begin
  x := !x;
end;
