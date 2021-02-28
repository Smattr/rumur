-- rumur_flags: CONFIG['SMT_BV_ARGS']
-- skip_reason: 'no SMT solver available' if CONFIG['SMT_BV_ARGS'] is None else None

-- equivalent of smt-forall4.m but using --smt-bitvectors on

var
  x: boolean;
  y: boolean;

startstate begin
  x := true;
end;

rule begin
  -- if the SMT bridge is working correctly, it should simplify the condition as
  -- a tautology into true, avoiding the read of an undefined variable
  if y | forall z := 2 to 4 by 2 do z = 2 | z = 4 end then
    x := !x;
  end;
end;
