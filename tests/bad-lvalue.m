-- rumur_exit_code: 1

/* This model deliberately uses something that shouldn't be assignable as the
 * LHS of an assignment, which should trigger an error.
 */

const
  N: 0;

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  N := 0;
  x := !x;
end;
