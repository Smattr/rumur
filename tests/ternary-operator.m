/* Initially due to an oversight, Rumur did not support the ternary operator.
 * This tests that we have not reintroduced that bug.
 */

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := x ? !x : !x;
end;
