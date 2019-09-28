-- rumur_flags: ['--bound', '3']

-- a basic test to confirm the --bound option works

var
  x: 0 .. 10;

startstate begin
  x := 0;
end;

rule begin
  x := 10 - x;
end;

rule x > 0 ==> begin
  x := x - 1;
end;

rule x < 10 ==> begin
  x := x + 1;
end;
