-- rumur_flags: ['--counterexample-trace', 'off']

/* This model tests for a similar bug to no-cex-bug.m. When a liveness property
 * was used in combination with no counterexample tracing, C code that failed to
 * compile would be produced.
 */

var
  x: 0 .. 9;

startstate begin
  x := 0;
end;

rule x > 0 ==> begin
  x := x - 1;
end;

rule x < 9 ==> begin
  x := x + 1;
end;

liveness x = 7;
