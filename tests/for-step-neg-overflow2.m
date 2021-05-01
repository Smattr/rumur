-- rumur_flags: ['--deadlock-detection', 'off', '--value-type', 'int32_t']

-- Identical to for-step-neg-overflow.m, but forcing a larger type used to
-- represent scalars to ensure this also works.

var
  x: -127 .. 0;

startstate begin
  x := 0;
end;

rule
  var counter: 0 .. 13;
begin
  counter := 0;
  for i := 0 to -125 by -10 do
    x := i;
    counter := counter + 1;
  end;
  -- extra paranoia: check the loop iterated the correct number of times
  assert counter = 13;
end;
