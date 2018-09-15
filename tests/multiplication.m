/* An early implementation of Rumur accidentally omitted support for
 * multiplication. This tests that we actually can parse multiplication.
 */

var
  x: 1 .. 10;

startstate begin
  x := 1;
end;

rule x > 1 ==> begin
  x := x - 1;
end;

rule x <= 5 ==> begin
  x := x * 2;
end;
