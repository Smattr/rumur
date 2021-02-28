-- rumur_flags: CONFIG['SMT_ARGS']
-- skip_reason: 'no SMT solver available' if CONFIG['SMT_ARGS'] is None else None

/* This model tests whether Rumur is capable of simplifying simple conditions at
 * code generation time. If it is, then it will replace the `y = y` check with
 * true and this model will pass. If not, the check will remain and cause a read
 * of an undefined value at runtime.
 */

var
  x: boolean;
  y: boolean;

startstate begin
  x := true;
end;

rule begin
  if y = y then
    x := !x;
  end;
end;
