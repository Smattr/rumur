-- rumur_exit_code: 1

-- test that defining a const after rules is rejected

var
  x: boolean;

startstate begin
  x := true;
end;

const N: 3;

rule begin
  x := !x;
end;
