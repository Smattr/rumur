-- rumur_flags: CONFIG['SMT_BV_ARGS']
-- skip_reason: 'no SMT solver available' if CONFIG['SMT_BV_ARGS'] is None else None

-- test that the SMT bridge can simplify left shifts when using bitvector logic

var
  x: 1 .. 2
  y: boolean

startstate begin
  y := true;
end

rule begin
  -- if the SMT bridge can handle left shift, it should simplify the following
  -- to `true`, avoiding an undefined read
  if x << 1 = 2 | x << 1 = 4 then
    y := !y;
  end;
end
