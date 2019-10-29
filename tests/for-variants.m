-- rumur_flags: ['--deadlock-detection', 'off']

-- this model tests a variety of for-loop expressions, as some of these have
-- proven problematic in the past

type
  t1: 0 .. 2;

var
  x: t1;
  y: t1;
  z: 0 .. 3;
  a: array[t1] of t1;

startstate begin

  -- loop by type
  for i: t1 do
    a[i] := i;
  end;

  assert a[0] = 0 & a[1] = 1 & a[2] = 2;

  undefine a;

  -- loop by inline type
  for i: 0 .. 2 do
    a[i] := i;
  end;

  assert a[0] = 0 & a[1] = 1 & a[2] = 2;

  undefine a;

  -- loop by range
  for i := 0 to 2 do
    a[i] := i;
  end;

  assert a[0] = 0 & a[1] = 1 & a[2] = 2;

  undefine a;

  -- loop that crosses 0
  for i := -1 to 1 do
    a[i + 1] := 1 - i;
  end;

  assert a[0] = 2 & a[1] = 1 & a[2] = 0;

  undefine a;

  -- range that ends at 0
  for i := -2 to 0 do
    a[-i] := -i;
  end;

  assert a[0] = 0 & a[1] = 1 & a[2] = 2;

  undefine a;

  -- range that is fully negative
  for i := -3 to -1 do
    a[3 + i] := 3 + i;
  end;

  assert a[0] = 0 & a[1] = 1 & a[2] = 2;

  undefine a;

  -- loop with a step
  for i := 0 to 2 by 1 do
    a[i] := i;
  end;

  assert a[0] = 0 & a[1] = 1 & a[2] = 2;

  undefine a;

  -- loop with a non-1 step
  for i := 0 to 2 by 2 do
    a[i] := i;
  end;

  assert a[0] = 0 & isundefined(a[1]) & a[2] = 2;

  undefine a;

  -- loop that does not start at 0
  for i := 1 to 2 do
    a[i] := i;
  end;

  assert isundefined(a[0]) & a[1] = 1 & a[2] = 2;

  undefine a;

  -- lower and upper bound equal
  for i := 0 to 0 do
    a[i] := i;
  end;

  assert a[0] = 0 & isundefined(a[1]) & isundefined(a[2]);

  undefine a;

  -- loop whose last step exceeds the upper bound
  for i := 0 to 2 by 3 do
    a[i] := i;
  end;

  assert a[0] = 0 & isundefined(a[1]) & isundefined(a[2]);

  undefine a;

  -- loop that has a variable lower bound
  x := 0;
  for i := x to 2 do
    a[i] := i;
  end;

  assert a[0] = 0 & a[1] = 1 & a[2] = 2;

  undefine a;

  -- loop that has a variable upper bound
  y := 2;
  for i := 0 to y do
    a[i] := i;
  end;

  assert a[0] = 0 & a[1] = 1 & a[2] = 2;

  undefine a;

  -- loop that has a variable lower and upper bound
  x := 0;
  y := 2;
  for i := x to y do
    a[i] := i
  end;

  assert a[0] = 0 & a[1] = 1 & a[2] = 2;

  undefine a;

  -- FIXME: the cases below currently fail

  -- loop with variable step
  z := 1;
  for i := 0 to 2 by z do
    a[i] := i;
  end;

  assert a[0] = 0 & a[1] = 1 & a[2] = 2;

  undefine a;

  -- loop with variable lower bound and step
  x := 0;
  z := 1;
  for i := x to 2 by z do
    a[i] := i;
  end;

  assert a[0] = 0 & a[1] = 1 & a[2] = 2;

  undefine a;

  -- loop with variable upper bound and step
  y := 2;
  z := 1;
  for i := 0 to y by z do
    a[i] := i;
  end;

  assert a[0] = 0 & a[1] = 1 & a[2] = 2;

  undefine a;

  -- loop with variable bounds and step
  x := 0;
  y := 2;
  z := 1;
  for i := x to y by z do
    a[i] := i;
  end;

  assert a[0] = 0 & a[1] = 1 & a[2] = 2;

  undefine a;

end;
