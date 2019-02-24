-- rumur_exit_code: 1

/* This model tests for the presence of a bug that was observed on commit
 * cf02a36f1c2f4a3726aba23aaa857ed90914ba54, wherein an enum type with duplicate
 * members is incorrectly accepted.
 */

type
  t: enum { A, A };

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;
