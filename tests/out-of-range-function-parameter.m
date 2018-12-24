-- checker_exit_code: 1
/* This model passes a different range into a non-var parameter (something that
 * should be allowed), but at runtime the value that is passed is outside the
 * range of the accepting parameter.
 */

var
  x: 0 .. 10;

procedure foo(y: 0 .. 5); begin
end;

startstate begin
  x := 3;
end;

rule begin
  x := 10 - x;
  foo(x);
end;
