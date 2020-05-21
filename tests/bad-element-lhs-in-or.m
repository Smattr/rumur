-- rumur_exit_code: 1

-- similar to bad-field-lhs-in-or.m, this model would cause a crash on commit
-- 427c4cb9424ef9ee065c1a3208639b94175bec54 due to invalid assumptions in
-- Element::type()

var
  x: 0 .. 10;

startstate begin
  x := x[0] | x;
end;
