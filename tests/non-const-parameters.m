/* As observed in commit eb74c568327d08b3619b94e80d6cd91464694f96, Rumur would
 * fail an assert when generating a model that passes the result of a function
 * to another function. The following model is designed to trigger this bug if
 * it is still present.
 */

type
  t: record
    x: boolean;
  end;

var
  x: boolean;

procedure foo(a: t); begin
end;

function bar(): t;
var
  a: t;
begin
  a.x := true;
  return a;
end;

startstate begin
  x := true;
end;

rule begin
  foo(bar());
  x := !x;
end;
