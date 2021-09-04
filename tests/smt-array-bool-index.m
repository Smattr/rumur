-- rumur_flags: CONFIG['SMT_ARGS']
-- skip_reason: 'no SMT solver available' if CONFIG['SMT_ARGS'] is None else None

/* test whether the SMT bridge can simplify expressions involving arrays with
 * boolean indices
 */

type
  t: array[boolean] of 0 .. 1;

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
  if x[true] = x[true] then
    y := !y;
  end;
end;
