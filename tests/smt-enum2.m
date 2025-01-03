-- rumur_flags: smt_args()
-- skip_reason: 'no SMT solver available' if smt_args() is None else None

-- slightly more involved version of smt-enum.m

var
  x: enum { A, B };
  y: boolean;

startstate begin
  y := true;
end;

rule begin
  -- if the SMT bridge can deal with enums, it should turn the following
  -- condition into `true`, avoiding a read of an undefined value from x
  if x = A | x = B then
    y := !y;
  end;
end;
