-- rumur_exit_code: 1

-- test that an illegaly out of section order reference within a function is
-- rejected

function foo(): boolean; begin
  return x;
end;

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;
