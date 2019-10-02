-- rumur_exit_code: 1

-- this model tests that a loop iterating in the wrong direction is rejected

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  -- this loop will never finish
  for i := 0 to 10 by -1 do
  end;
  x := !x;
end;
