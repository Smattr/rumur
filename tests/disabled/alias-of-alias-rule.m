-- This tests for regression of a bug reported against Rumur v2020.03.12.
-- Alias rules would have their contained aliases emitted in reverse order
-- during code generation. This meant that aliases that referenced earlier
-- aliases would be emitted referencing a variable that had not yet been
-- defined. If this bug has been reintroduced, this model will pass code
-- generation but then the generated code will fail to compile.

var
  x: boolean;

startstate begin
  x := true;
end;

alias
  -- these two lines will have their code generated in swapped order if the bug
  -- has been reintroduced
  y: x;
  z: y;
do
  rule begin
    z := !z;
  end;
end;
