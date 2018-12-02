-- Test of a nested switch statement

var
  x: 0 .. 10;

startstate begin
  x := 5;
end;

rule begin

  switch x

    case 0, 1, 2, 3, 4, 5:
      switch x

        case 0:
          x := x + 1;

        else
          x := x - 1;

      end;

    else
      x := 10 - x;

  end;

end;

rule begin
  x := 10 - x;
end;
