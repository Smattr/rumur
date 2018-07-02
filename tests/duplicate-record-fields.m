-- rumur_exit_code: 1

-- Test for duplicate field names within a record. This should fail to generate
-- a checker.

type
  foo_t: record
    a: boolean;
    a: 0 .. 2;
  end;

var
  x: boolean;
  y: foo_t;

startstate begin
  x := true;
  y.a := true;
end;

rule begin
  x := !x;
end;
