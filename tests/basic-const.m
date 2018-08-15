const
  N: 2;

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  if N = 2 then
    x := !x;
  end;
end;
