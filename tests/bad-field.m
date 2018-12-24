-- rumur_exit_code: 1
/* The following model should be rejected due to the use of 'x' as if it were a
 * record. However, on commit c65737161d9151d8a69ad718aea370636ff73829 it was
 * observed that this actually causes an uncaught exception instead. This was
 * originally found by AFL.
 */

type
  t: enum { A, B };

var
  x: t;

startstate begin
  x := A;
end;

rule x.A ==> begin
  x := B;
end;
