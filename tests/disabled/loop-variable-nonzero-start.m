-- This model tests for the presence of a bug first observed on commit
-- fa71679f33aec0cceafb7528d24c03e3b2d315f3. The limits used for unpacking the
-- handle related to the loop variable were incorrect, causing loops like the
-- one in the following model to turn into no-ops. If this bug has been
-- reintroduced, this model will deadlock.

var
  x: 0 .. 10;

startstate begin
  x := 0;
end;

rule x = 0 ==> begin
  for y := 5 to 9 do
    x := x + 1;
  end;
end;

rule x = 5 ==> begin
  x := 0;
end;

invariant x = 0 | x = 5;
