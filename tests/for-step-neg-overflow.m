-- rumur_flags: ['--deadlock-detection', 'off']

-- This model is designed to trigger a signed integer overflow in generated code
-- if a particular bug has been reintroduced. Based on the numeric literals
-- appearing in this model, Rumur should decide it is OK to use int8_t to
-- represent scalar values. However, a naïve translation of the for loop will
-- result in i temporarily having the value -130, outside the range of int8_t.
--
-- Rumur should handle this gracefully, but previously did not. If this issue
-- has been reintroduced, this model will result in generated code that causes
-- compiler warnings or reads undefined values from i at runtime.

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
