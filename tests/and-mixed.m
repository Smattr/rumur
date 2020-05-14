-- rumur_exit_code: 1

-- test that & with mixed operands is rejected

var
  x: 0 .. 10
  y: boolean

startstate begin
  y := true;
end

rule begin

  x := 2 & true;

  y := !y;
end
