-- rumur_flags: ['--smt-simplification', 'on'] + smt_args()
-- skip_reason: 'no SMT solver available' if len(smt_args()) == 0 else None

-- similar to smt-forall, but using a pre-defined type

type
  t: 1 .. 2;

var
  x: boolean;
  y: boolean;

startstate begin
  x := true;
end;

rule begin
  -- if the SMT bridge is working correctly, it should simplify the condition as
  -- a tautology into true, avoiding the read of an undefined variable
  if forall z: t do z = 1 | z = 2 end | y then
    x := !x;
  end;
end;
