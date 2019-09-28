-- rumur_flags: ['--bound', '3']
-- checker_exit_code: 1

-- test that we can still violate an invariant that is *just* within the --bound

var
  x: 0 .. 10;

startstate begin
  x := 0;
end;

rule x < 10 ==> begin
  x := x + 1;
end;

rule x > 0 ==> begin
  x := x - 1;
end;

invariant "x stays small" x < 3;
