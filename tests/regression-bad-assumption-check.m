-- rumur_flags: ['--deadlock-detection', 'stuck']

/* There was a previous bug wherein assumption checks were done on the preceding
 * state of a rule, rather than on the final state. This model tests that we
 * have not re-introduced this bug.
 */

var
  x: boolean;

startstate
  x := true;
end;

rule begin
  x := !x;
end;

rule begin
  x := x;
end;

-- This assumption should guarantee that the invariant below it never triggers.
assume x
invariant x
