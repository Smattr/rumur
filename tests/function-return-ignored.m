/* Test calling a function as if it were a procedure (function that does not
 * return a value). Note, CMurphi does not support this extension.
 */
var
  x: boolean;

function foo(): boolean; begin
  return true;
end;

startstate begin
  x := true;
end;

rule begin
  x := !x;
  foo();
end;
