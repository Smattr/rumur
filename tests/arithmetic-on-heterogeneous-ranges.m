/* Tests that arithmetic between two values of differing range types is
 * supported.
 */
var
  x: 1..10;
  y: 1..5;

startstate begin
  x := 1;
  y := 1;
end;

rule x > 1 ==> begin
  x := x - y;
end;

rule x < 10 ==> begin
  x := x + y;
end;
