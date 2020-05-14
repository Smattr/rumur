-- rumur_exit_code: 1

-- test that & on an enum is rejected

var
  x: enum { A, B }
  y: boolean

startstate begin
  y := true;
end

rule begin

  x := A & B;

  y := !y;
end
