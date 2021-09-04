-- rumur_flags: CONFIG['SMT_ARGS']
-- skip_reason: 'no SMT solver available' if CONFIG['SMT_ARGS'] is None else None

-- As observed on commit 852de6485322fe6e0dfaa8efa0109f23f634bf3f, models with
-- negative literal values would cause malformed SMT problems to be constructed.
-- This model tests that this bug has not been reintroduced. Note that the
-- negative literal does not actually need to be used in the model for the bug
-- to manifest.

const
  N: -10;

var
  x: boolean;
  y: boolean;

startstate begin
  y := true;
end;

rule begin
  -- the SMT bridge should be able to optimise the following conditional into
  -- `true` preventing read of an undefined value
  if x | !x then
    y := !y;
  end;
end;
