-- rumur_exit_code: 1
/* This test contains an ID expression that incorrectly refers to a typedecl.
 * This should result in an error.
 */

type
  t: boolean;

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := t;
end;
