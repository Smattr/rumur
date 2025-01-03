-- rumur_flags: smt_bv_args()
-- skip_reason: 'no SMT solver available' if smt_bv_args() is None else None

-- equivalent of smt-exists.m but using --smt-bitvectors on

var
  x: boolean;
  y: boolean;

startstate begin
  x := true;
end;

rule begin
  -- if the SMT bridge is working correctly, it should simplify the condition as
  -- a tautology into true, avoiding the read of an undefined variable
  if y | exists z: 1 .. 2 do z = 1 end then
    x := !x;
  end;
end;
