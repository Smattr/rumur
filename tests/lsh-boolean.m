-- rumur_exit_code: 1

-- test that left shifting a boolean is rejected

var
  x: 0 .. 10
  y: boolean

startstate begin
  x := 0;
end

rule begin
  x := y << 1;
end
