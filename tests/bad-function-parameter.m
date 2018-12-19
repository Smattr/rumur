-- rumur_exit_code: 1
/* This model is designed to provoke a bug observed on commit
 * ad4078683bd8737add75a294fec65d8bd2f6b84b, wherein passing a boolean literal
 * in as a var parameter to a function caused an uncaught exception. Rumur
 * should correctly detect and handle this case, but if the bug has been
 * reintroduced an uncaught exception will occur during code generation.
 */

var
  x: boolean;

procedure foo(var y: boolean); begin
end;

startstate begin
  x := true;
end;

rule begin
  foo(true);
  x := !x;
end;
