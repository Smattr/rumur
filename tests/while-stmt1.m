-- Basic test that while statements work

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin

  -- While statement using a state variable
  while x do
    x := !x;
  end;

  -- While statement that uses an expression (non-lvalue)
  while (x | !x) & !x do
    x := !x;
  end;

  -- While statement using a constant
  while false do
    x := !x;
  end;

end;

rule begin
  x := !x;
end;
