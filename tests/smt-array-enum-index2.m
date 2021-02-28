-- rumur_flags: CONFIG['SMT_ARGS']
-- skip_reason: 'no SMT solver available' if CONFIG['SMT_ARGS'] is None else None

-- variant of smt-array-enum-index.m that pre-defines the enum type

type
  e: enum { A, B };
  t: array[e] of 0 .. 1;

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
  if x[A] = x[A] then
    y := !y;
  end;
end;
