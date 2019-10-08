-- this model tests that we can write loops that iterate backwards

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  for i := 9 to 1 by -1 do
    x := !x;
  end;
end;
