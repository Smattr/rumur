-- Basic test of a switch statement

var
  x: 0 .. 10;
  y: boolean;
  z: enum { A, B, C };

startstate begin
  x := 1;
  y := true;
  z := A;
end;

rule begin

  -- Switch on a state variable
  switch x

    case 1:
      x := x + 1;

    -- Empty case
    case 2:

    -- Multiple match
    case 3, 4:
      x := x + 1;

    else
      x := 10 - x;

  end;

  -- Switch on a non-lvalue
  switch 10 - x

    case 1:
      x := x - 1;

    else
      x := 10 - x;

  end;

  -- Switch with no else
  switch x
    case 1:
      x := x + 1;
  end;

  -- Switch with no cases
  switch x
  end;

  -- Switch on literal
  switch 10

    case 3:
      x := 10 - x;

    case 10:
      x := 10 - x;

  end;

  -- Switch on boolean
  switch y

    case false:
      y := !y;

    case true:
      y := !y;

  end;

  -- Switch on boolean literal
  switch true

    case false:
      y := !y;

    case true:
      y := !y;

  end;

  -- Switch on enum
  switch z

    case A:
      z := A;

    case B:
      z := B;

    case C:
      z := C;

    -- Note, this is unreachable but should still be allowed
    else
      z := A;

  end;

  -- Switch on enum literal
  switch A

    case A:
      z := A;

    case B:
      z := B;

  end;

end;

rule begin
  x := 10 - x;
end;
