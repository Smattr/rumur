-- rumur_exit_code: 1
/* The following model should fail code generation due to the invalid array
 * index. However, in commit c65737161d9151d8a69ad718aea370636ff73829 it was
 * observed that this actually causes an assertion failure. This issue was
 * originally found by AFL.
 */
type
  foo_t: array[0 .. 1] of boolean;

var
  x: foo_t;

startstate begin
  x[0][0] := x[1];
end;
