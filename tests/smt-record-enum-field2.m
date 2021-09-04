-- rumur_flags: CONFIG['SMT_ARGS']
-- skip_reason: 'no SMT solver available' if CONFIG['SMT_ARGS'] is None else None

-- variant of smt-record-enum-field.m that pre-defines the enum type

type
  e: enum { A, B };
  t: record
    x: e;
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
