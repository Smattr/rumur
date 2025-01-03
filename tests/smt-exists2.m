-- rumur_flags: smt_args()
-- skip_reason: 'no SMT solver available' if smt_args() is None else None

-- similar to smt-exists, but using a pre-defined type

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
  if y | exists z: t do z = 1 end then
    x := !x;
  end;
end;
