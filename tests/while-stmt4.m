-- rumur_exit_code: 1

-- Test that ranges in while loops are rejected

var
  x: 0 .. 10;

startstate begin
  x := 5;
end;

rule begin

  while x do
    x := 6 - x;
  end;

end;
