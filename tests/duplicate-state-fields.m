-- rumur_exit_code: 1

-- Test for duplicate names within the state. This should fail to generate a
-- checker.

var
  a: boolean;
  a: 0 .. 2;

startstate begin
  a := 0;
end;

rule begin
  a := 0;
end;
