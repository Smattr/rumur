-- rumur_exit_code: 1

-- test that an illegal variable reference out of section order is rejected

var
  x: boolean;

startstate begin
  x := true;
  y := true;
end;

var
  y: boolean;

rule begin
  x := !x;
end;
