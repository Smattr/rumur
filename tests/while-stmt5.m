-- rumur_exit_code: 1

-- Test that complex types in while expressions are rejected

var
  x: record
    y: boolean;
  end;

startstate begin
  x.y := true;
end;

rule begin

  while x do
    x.y := !x.y;
  end;

end;
