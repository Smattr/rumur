type
  range_t: 0 .. 1;
  array_t: array[range_t] of range_t;

var
  x: range_t;

startstate begin
  x := 0;
end;

rule begin
  x := 1 - x;
end;
