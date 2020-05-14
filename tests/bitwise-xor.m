-- test that bitwise XOR works as expected

var
  x: 0 .. 5
  y: boolean

startstate begin
  y := true;
end

rule begin

  x := 1 ^ 2;
  assert x = 3;

  x := 2 ^ 3;
  assert x = 1;

  x := 2 ^ 0;
  assert x = 2;

  x := 3 ^ 0;
  assert x = 3;

  y := !y;
end
