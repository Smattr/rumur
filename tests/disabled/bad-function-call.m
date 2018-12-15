-- rumur_exit_code: 1
/* Rumur should reject the following model due to the incorrect call of function
 * foo. However, on commit c65737161d9151d8a69ad718aea370636ff73829 it was
 * observed that this actually caused an assertion failure. This was originally
 * found by AFL.
 */
var
  x: boolean;

function foo(a: boolean): boolean; begin
  return a;
end;

startstate begin
  x := true;
end;

rule begin
  x := !foo(x, x);
end;
