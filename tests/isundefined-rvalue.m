-- rumur_exit_code: 1
-- Test of isundefined on rvalues, that should not work.

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := isundefined(true);
end;
