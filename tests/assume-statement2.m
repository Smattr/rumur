/* Test using an assumption that fails as a statement. This was observed to
 * cause a segfault or assertion failure on
 * 138877410d739f1d9fc07ee8f1107f8f3c5676de.
 */

var
  x: 0 .. 2

startstate begin
  x := 0;
end

rule x > 0 ==> begin
  x := x - 1;
end

rule x < 2 ==> begin
  x := x + 1;
  assume x != 2;
end

-- if the assumption is working correctly, this invariant should pass
invariant x != 2
