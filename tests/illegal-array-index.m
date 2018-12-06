-- rumur_exit_code: 1

-- This was an illegal model AFL found that caused a crash using commit
-- 5ccd2be57847aa173a4741340b359edcc3b54074.

type
  -- Note: array index bounds are back to front
  foo_t: array[8 .. 1] of boolean;

var
  x: foo_t;

startstate begin
  x[0] := true;
  x[1] := false;
end;

rule begin
  x[0] := x[1];
  x[1] := x[0];
end;
