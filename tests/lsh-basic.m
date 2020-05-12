-- basic test of left shift operator (Rumur extension)

var
  x: 0 .. 10
  y: 0 .. 4
  z: boolean

startstate begin
  x := 0;
  z := true;
end

rule begin

  assert x = 0;

  -- left shift of 0 should still yield 0
  x := x << 0;
  assert x = 0 "<< 0 yielded wrong value";
  x := x << 1;
  assert x = 0 "<< 1 yielded wrong value";
  y := 2;
  x := x << y;
  assert x = 0 "<< 2 via variable yielded wrong value";
  x := x << -1;
  assert x = 0 "<< -1 yielded wrong value";
  x := x << -4;
  assert x = 0 "<< -4 yielded wrong value";

  -- some general shift cases
  x := 1 << 2;
  assert x = 4;
  x := 2 << 1;
  assert x = 4;
  x := 2 << 2;
  assert x = 8;

  -- overshifts should always produce 0
  x := 0 << 128;
  assert x = 0;
  x := 1 << 128;
  assert x = 0;
  x := 1 << -128;
  assert x = 0;
  x := 2 << 128;
  assert x = 0;
  x := y << 128;
  assert x = 0;

  z := !z;
end
