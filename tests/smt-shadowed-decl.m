-- rumur_flags: CONFIG['SMT_ARGS']
-- skip_reason: 'no SMT solver available' if CONFIG['SMT_ARGS'] is None else None

-- This model tests a scenario where one declaration shadows another during
-- translation across the SMT bridge. If everything works correctly, Rumur
-- should form a valid SMT problem and the SMT solver should find that the
-- condition in the if statement is a tautology and reduce it to "true",
-- removing the read of an undefined variable. However, on commit
-- 4ff47d10ee40a4947c3ee0463fddbf6f0fee1857 it was observed that this generates
-- an invalid SMT problem.

var
  x: boolean;
  y: boolean;

startstate begin
  y := true;
end;

rule
  var x: boolean;
begin
  if x | !x then
    y := !y;
  end;
end;
