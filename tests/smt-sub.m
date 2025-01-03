-- rumur_flags: smt_args()
-- skip_reason: 'no SMT solver available' if smt_args() is None else None

-- test that the SMT bridge can cope with subtraction

var
  x: 8 .. 9;
  y: boolean;

startstate begin
  y := true;
end;

rule begin
  -- the following condition should be simplified to `true` avoiding a read of
  -- `x` when it is undefined
  if x - 1 = 7 | x - 1 = 8 then
    y := !y;
  end;
end;
