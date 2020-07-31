-- test of an alias of an alias in an alias statement

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  alias
    y: x;
    z: y;
  do
    z := !z;
  end;
end;
