-- Test of switch statement with non-constant cases. This is a Rumur extension.

var
  x: 0 .. 10;
  y: 0 .. 10;

startstate begin
  x := 0;
  y := 1;
end;

rule begin

  switch x

    case y:
      x := 10 - x;

    case 5:
      x := 10 - x;

  end;

end;

rule begin
  x := 10 - x;
  y := 10 - y;
end;
