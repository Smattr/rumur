-- rumur_flags: smt_args()
-- skip_reason: 'no SMT solver available' if smt_args() is None else None

-- variant of smt-array-enum-value-and-index.m which pre-defines the enums

type
  e1: enum { C, D };
  e2: enum { A, B };
  t: array[e1] of e2;

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
  if x[C] = x[C] then
    y := !y;
  end;
end;
