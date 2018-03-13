/* A model that uses an array with a boolean type indexing it. */

type
  bar_t: 0 .. 1;
  foo_t: array [boolean] of bar_t;

var
  x: foo_t;

startstate begin
  x[false] := 0;
  x[true] := 1;
end;

rule begin
  x[false] := 0;
  x[true] := 1;
end;

rule begin
  x[false] := 1;
  x[true] := 0;
end;
