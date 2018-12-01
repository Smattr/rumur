-- Test of a while statement using ranges

var
  x: 0 .. 10;

startstate begin
  x := 1;
end;

rule begin

  -- While statement using a state variable
  while x + 1 < 10 do
    x := x + 1;
  end;

  -- While statement using a constant expression
  while 1 + 1 > 10 do
  end;

  x := 5;

end;

rule begin
  x := 6 - x;
end;
