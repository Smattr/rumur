/* On commit 14de19ce3bb545ddb4857fd91cc3a17a4fa2fccb, it was observed that
 * scalar values being passed to non-var function parameters were being
 * calculated incorrectly. This model checks that this bug has not been
 * reintroduced.
 */

var
  x: 1 .. 3;

procedure foo(y: 1 .. 3); begin
  assert y < 3;
end;

startstate begin
  x := 1;
end;

rule begin
  foo(x);
  x := 2 - (x - 1);
end;
