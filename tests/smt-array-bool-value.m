-- rumur_flags: smt_args()
-- skip_reason: 'no SMT solver available' if smt_args() is None else None

/* test whether the SMT bridge can simplify expressions involving arrays with
 * boolean values
 */

type
  t: array[0 .. 1] of boolean;

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
  if x[0] = x[0] then
    y := !y;
  end;
end;
