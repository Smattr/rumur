-- checker_exit_code: 1
-- checker_output: None if xml else re.compile('enum { HELLO, WORLD }')

/* This model attempts to provoke a bug first observed on commit
 * 42b424c6d5986e44c218152e96b065b056ea494b. Enums would be incorrectly printed
 * without separating spaces. If this bug has been reintroduced, the following
 * model will fail with an assertion message that says something incorrect like
 * "enum { HELLOWORLD }".
 */

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
  assert forall y: enum { HELLO, WORLD } do false end;
end;
