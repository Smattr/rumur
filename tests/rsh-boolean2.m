-- rumur_exit_code: 1

-- test that right shifting by a boolean is rejected

var
  x: 0 .. 10
  y: boolean

startstate begin
  x := 0;
  y := true;
end

rule begin
  x := 1 >> y;
end
