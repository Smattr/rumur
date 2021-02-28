-- rumur_flags: CONFIG['SMT_BV_ARGS']
-- skip_reason: 'no SMT solver available' if CONFIG['SMT_BV_ARGS'] is None else None

-- test that the SMT bridge can deal with bitwise NOT

var
  x: 0 .. 10
  y: boolean

startstate begin
  y := true;
end

rule begin
  -- the following condition should be simplified to `true` avoiding a read of
  -- `x` when it is undefined
  if ~~x = x then
    y := !y;
  end;
end
