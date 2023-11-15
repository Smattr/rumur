-- test of unicode multiplication symbol

var
  x: 1 .. 2;

startstate begin
  x := 1;
end;

rule begin
  if x = 2 then
    x := 1;
  else
    x := 2 × x;
  end;
end;
