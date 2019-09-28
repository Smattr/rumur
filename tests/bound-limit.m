-- rumur_flags: ['--bound', '3']

-- test a --bound that should prevent an invariant being violated

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

invariant "x stays small" x < 4;
