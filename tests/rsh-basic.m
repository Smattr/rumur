-- basic test of right shift operator (Rumur extension)

var
  x: -10 .. 10
  y: 0 .. 4
  z: boolean

startstate begin
  z := true;
end

rule begin

  x := 0;

  -- right shift of 0 should still yield 0
  x := x >> 0;
  assert x = 0 ">> 0 yielded wrong value";
  x := x >> 1;
  assert x = 0 ">> 1 yielded wrong value";
  y := 2;
  x := x >> y;
  assert x = 0 ">> 2 via variable yielded wrong value";
  x := x >> -1;
  assert x = 0 ">> -1 yielded wrong value";
  x := x >> -4;
  assert x = 0 ">> -4 yielded wrong value";

  -- some general shift cases
  x := 1 >> 1;
  assert x = 0;
  x := 2 >> 1;
  assert x = 1;
  x := 4 >> 2;
  assert x = 1;

  -- overshifts should always produce 0
  x := 0 >> 128;
  assert x = 0;
  x := 1 >> 128;
  assert x = 0;
  x := 1 >> -128;
  assert x = 0;
  x := 2 >> 128;
  assert x = 0;
  x := y >> 128;
  assert x = 0;

  -- some negative shift cases
  x := -1 >> 1;
  assert x = -1;
  x := -3 >> 1;
  assert x = -2;

  z := !z;
end
