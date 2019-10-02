-- rumur_exit_code: 1

-- this model tests that a trivial infinite loop is rejected

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  -- this loop will never finish
  for i := 0 to 10 by 0 do
  end;
  x := !x;
end;
