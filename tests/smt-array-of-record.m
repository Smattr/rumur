-- rumur_flags: smt_args()
-- skip_reason: 'no SMT solver available' if smt_args() is None else None

-- test the SMT bridge can cope with arrays of records

var
  x: array[0 .. 1] of record x: 0 .. 1; end;
  y: boolean;

startstate begin
  y := true;
end;

rule begin
  -- if the SMT bridge is working correctly, it will simplify the following
  -- expression to 'true' avoiding the read of an undefined variable
  if x[0].x = x[0].x then
    y := !y;
  end;
end;
