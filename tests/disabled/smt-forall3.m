-- rumur_flags: SMT_ARGS
-- skip_reason: 'no SMT solver available' if len(SMT_ARGS) == 0 else None

-- a variant of smt-forall.m using an inline range

var
  x: boolean;
  y: boolean;

startstate begin
  x := true;
end;

rule begin
  -- if the SMT bridge is working correctly, it should simplify the condition as
  -- a tautology into true, avoiding the read of an undefined variable
  if forall z := 1 to 2 do z = 1 | z = 2 end | y then
    x := !x;
  end;
end;
