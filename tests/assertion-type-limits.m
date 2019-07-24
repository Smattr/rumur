/* This model is designed to provoke a case where the size of the generated
 * state is 31 bits. On 529f40ddfff9de06c716b093dd229e754940ac9b it was observed
 * that this generates code in debug mode that generates a compiler warning when
 * built with -Wtype-limits.
 */

type
  foo_t: 0 .. 1073741824;

var
  x: foo_t;

startstate begin
  x := 0;
end;

rule begin
  x := 1 - x;
end;
