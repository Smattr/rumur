-- rumur_exit_code: 1

-- On commit 427c4cb9424ef9ee065c1a3208639b94175bec54, AFL found a crashing
-- input that could be reduced to something like the following. During symbol
-- resolution, Field::type() would be called while trying to disambiguate the
-- '|'. However Field::type() was assuming symbol resolution had already
-- completed successfully and thus it was safe to assume its left hand side was
-- a record. In this example, the field expression is malformed because the left
-- hand side is not a record. If this bug has been re-introduced, this model
-- should cause an assertion failure or crash instead of correctly being
-- rejected by Rumur.

var
  x: 0 .. 10;

startstate begin
  x := x.y | x;
end;
