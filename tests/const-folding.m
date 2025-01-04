-- test that constant folding can deal with non-trivial expressions

function f(): 0 .. 1 begin
  return 0;
end;

var
  x: 0 .. false ? f() : 1;

startstate begin
  x := 0;
end;

rule begin
  x := 1 - x;
end;
