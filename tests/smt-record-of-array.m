-- rumur_flags: CONFIG['SMT_ARGS']
-- skip_reason: 'no SMT solver available' if CONFIG['SMT_ARGS'] is None else None

-- test the SMT bridge can cope with records of arrays

var
  x: record x: array[0 .. 1] of 0 .. 1; end;
  y: boolean;

startstate begin
  y := true;
end;

rule begin
  -- if the SMT bridge is working correctly, it will simplify the following
  -- expression to 'true' avoiding the read of an undefined variable
  if x.x[0] = x.x[0] then
    y := !y;
  end;
end;
