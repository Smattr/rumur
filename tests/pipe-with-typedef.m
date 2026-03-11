-- rumur_exit_code: 1

/* This test case checks for a problem that was previously observed, wherein a
 * `|` operator would not be resolved into a bitwise OR if the type of the LHS
 * was a typedef rather than a raw range.
 */

type
  foo_t: 0..1;

var
  x: foo_t;

startstate begin
  x := 0;
end;

rule begin
  x := 1 - (x | x);
end;
