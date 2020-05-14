-- test that & is usable as bitwise and

var
  x: 0 .. 5
  y: boolean

startstate begin
  y := true;
end

rule begin

  x := 1 & 2;
  assert x = 0;

  x := 2 & 3;
  assert x = 2;

  y := !y;
end
