-- rumur_exit_code: 1

/* A liveness property makes no sense in a statement (as opposed to the top
 * level) and should be rejected.
 */

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
  liveness "not allowed" x | !x;
end;
