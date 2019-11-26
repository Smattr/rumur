-- rumur_flags: smt_args(True)
-- skip_reason: 'no SMT solver available' if len(smt_args()) == 0 else None

-- test that the SMT bridge can cope with multiplication when using a bitvector logic

var
  x: 8 .. 9;
  y: boolean;

startstate begin
  y := true;
end;

rule begin
  -- the following condition should be simplified to `true` avoiding a read of
  -- `x` when it is undefined
  if x * 5 = 40 | x * 5 = 45 then
    y := !y;
  end;
end;
