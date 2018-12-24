-- rumur_exit_code: 1
/* This model should be rejected due to the incorrect usage of 'y' within the
 * alias block. However, on commit c65737161d9151d8a69ad718aea370636ff73829 it
 * was observed that this actually causes an assertion failure instead. This was
 * originally found by AFL.
 */

var
  x: boolean;

startstate begin
end;

rule begin
  alias y: 1 do
    y := !y.x;
  end;
end;
