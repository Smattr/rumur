-- rumur_flags: CONFIG['SMT_ARGS']
-- skip_reason: 'no SMT solver available' if CONFIG['SMT_ARGS'] is None else None

-- slight variant on smt-record-range-field.m with a pre-defined type

type
  r: 0 .. 1;
  t: record
    x: r;
  end;

var
  x: t;
  y: boolean;

startstate begin
  y := true;
end;

rule begin
  /* if the SMT bridge is working correctly, it will simplify the condition in
   * the following expression to `true` avoiding the read of an undefined
   * variable
   */
  if x.x = x.x then
    y := !y;
  end;
end;
