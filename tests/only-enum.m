-- rumur_flags: ['--deadlock-detection', 'stuck']

type
  foo_t: enum { A, B };

var
  x: foo_t;

startstate begin
  x := A;
end;

rule begin
  x := B;
end;
