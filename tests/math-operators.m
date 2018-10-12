-- Test support for some of the alternative operators

var
  x: 0 .. 10;

startstate begin
  x := 0;
end;

rule
  x = 0 ∨ x = 1
==> begin
  x := x + 1;
end;

rule
  ¬(x = 0) ∧ ¬(x = 1)
==> begin
  x := x - 1;
end;

invariant
  x = 0 ∨ ¬(x = 0)
