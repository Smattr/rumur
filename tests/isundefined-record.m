-- rumur_exit_code: 1
-- Test of isundefined on records, that should not work.

var
  x: record
    y: boolean;
  end;

startstate begin
  x.y := true;
end;

rule begin
  x.y := isundefined(x);
end;
