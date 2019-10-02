-- rumur_exit_code: 1

-- this model tests that infinite down-loops are rejected

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  -- this loop will never finish
  for i := 10 to 0 by 1 do
  end;
  x := !x;
end;
