-- rumur_flags: CONFIG['SMT_ARGS']
-- skip_reason: 'no SMT solver available' if CONFIG['SMT_ARGS'] is None else None

/* This model attempts to provoke a bug first observed on commit
 * 5d4f1939ddc5d5d9336f0ce35e953c51e8b5aeca. The SMT bridge did not correctly
 * understand the boolean literals and would form malformed problems when seeing
 * them. If this problem has been introduced, this model will fail verification
 * by reading an undefined variable.
 */

var
  x: boolean;
  y: boolean;

startstate begin
  x := true;
end;

rule begin
  /* if the SMT bridge correctly deals with boolean literals, the following
   * expression should be simplified to true and avoid the access to an
   * undefined variable
   */
  if y = true | y = false then
    x := !x;
  end;
end;
