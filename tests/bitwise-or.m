-- test that | is usable as bitwise OR

var
  x: 0 .. 5
  y: boolean

startstate begin
  y := true;
end

rule begin

  x := 1 | 2;
  assert x = 3;

  x := 2 | 3;
  assert x = 3;

  x := 2 | 0;
  assert x = 2;

  y := !y;
end
