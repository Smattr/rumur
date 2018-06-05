type
  foo_t: array[0 .. 1] of boolean;

var
  x: foo_t;

startstate begin
  x[0] := true;
  x[1] := false;
end;

rule begin
  x[0] := false;
  x[1] := true;
end;

rule begin
  x[0] := true;
  x[1] := false;
end;
