-- rumur_flags: SMT_ARGS
-- skip_reason: 'no SMT solver available' if len(SMT_ARGS) == 0 else None

/* This is an example that contains a tautology. The SMT bridge should be able
 * to simplify this to `true`, but it currently cannot. The cause is that we
 * over-approximate ranges used as array indexes and values. So the SMT bridge
 * does not know that the values of the array can only be 1.
 */

var
  x: array[0 .. 1] of 1 .. 1;
  y: array[0 .. 1] of 1 .. 1;
  z: boolean;

startstate begin
  z := true;
end;

rule begin
  if x = y then
    z := !z;
  end;
end;
