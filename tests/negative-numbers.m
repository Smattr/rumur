const
  N: -2;

type
  r1: -1 .. 1;
  r2: -3 .. -2;
  r3: N .. 1;

var
  x: r3;

startstate begin
  x := N;
end;

rule begin
  if x = N then
    x := -1;
  else
    x := N;
  end;
end;
