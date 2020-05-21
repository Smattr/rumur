-- rumur_exit_code: 1

-- test that right shifting an enum is rejected

type
  t: enum { A, B }

var
  x: t

startstate begin
  x := A;
end

rule begin
  x := a >> 1;
end
