-- rumur_flags: ['--smt-simplification', 'on'] + smt_args()
-- skip_reason: 'no SMT solver available' if len(smt_args()) == 0 else None

-- test whether the SMT bridge can simplify expressions involving records with
-- range fieldss

type
  t: record
    x: 0 .. 1;
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
