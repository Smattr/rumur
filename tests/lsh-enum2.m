-- rumur_exit_code: 1

-- test that left shifting by an enum is rejected

type
  t: enum { A, B }

var
  x: 0 .. 10

startstate begin
  x := 0;
end

rule begin
  x := 2 << A;
end
