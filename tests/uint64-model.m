/* this model is designed to require values out of the range of int64_t to test
 * uint64_t is usable
 */

var
  x: 0 .. 0xfffffffffffffffe;

startstate begin
  x := 0;
end;

rule begin
  x := 0xfffffffffffffffe - x;
end;
