-- Test that while statements work with record-nested fields

var
  x: record
    y: boolean;
  end;

  z: record
    w: 0 .. 10;
  end;

startstate begin
  x.y := true;
  z.w := 1;
end;

rule begin

  while x.y do
    x.y := !x.y;
  end;

  while z.w < 2 do
    z.w := 6 - z.w;
  end;

end;

rule begin
  x.y := !x.y;
  z.w := 6 - z.w;
end;
