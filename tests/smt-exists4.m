-- rumur_flags: CONFIG['SMT_ARGS']
-- skip_reason: 'no SMT solver available' if CONFIG['SMT_ARGS'] is None else None

-- a variant of smt-exists.m using an inline range with stepping

var
  x: boolean;
  y: boolean;

startstate begin
  x := true;
end;

rule begin
  -- if the SMT bridge is working correctly, it should simplify the condition as
  -- a tautology into true, avoiding the read of an undefined variable
  if y | !exists z := 2 to 4 by 2 do z = 3 end then
    x := !x;
  end;
end;
