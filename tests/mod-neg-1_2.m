-- rumur_flags: ['--value-type=int8_t']

-- `VALUE_MIN % -1` should be computable without overflow

var
  x: 0 .. 1;

startstate begin
  x := 1;
end;

rule begin
  x := 1 - x + -128 % -1;
end;
