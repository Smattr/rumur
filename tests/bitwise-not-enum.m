-- rumur_exit_code: 1

-- confirm bitwise NOT is not accepted on non-range types

type
  t: enum { A, B }

var
  x: t
  y: boolean

startstate begin
  y := true;
end

rule begin
  x := ~A;
  y := !y;
end
