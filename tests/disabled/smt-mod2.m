-- rumur_flags: SMT_ARGS
-- skip_reason: 'no SMT solver available' if len(SMT_ARGS) == 0 else None

-- test that the SMT bridge can cope with modulo by a non-constant

var
  x: 8 .. 9;
  y: boolean;

startstate begin
  y := true;
end;

rule begin
  -- the following condition should be simplified to `true` avoiding a read of
  -- `x` when it is undefined
  if x % x = 0 then
    y := !y;
  end;
end;
