-- rumur_flags: ['--smt-simplification', 'on'] + smt_args() + ['--smt-logic', 'AUFBV', '--smt-bitvectors', 'on']
-- skip_reason: 'no SMT solver available' if len(smt_args()) == 0 else None

-- test that the SMT bridge can cope with division when using a bitvector logic

var
  x: 4 .. 6;
  y: boolean;

startstate begin
  y := true;
end;

rule begin
  -- the following condition should be simplified to `true` avoiding a read of
  -- `x` when it is undefined
  if x / 2 = 2 | x / 2 = 3 then
    y := !y;
  end;
end;
