/* This model is intended to provoke a bug that was observed on commit
 * 7eafb5295a1f2f094579ef259b537bd3e4996158. Constants would always be
 * constructed with no type (i.e. implicitly an unrestricted range). This is
 * fine for anything except a boolean (non-range) constant. A boolean constant
 * that is constructed with no type can never be used in a boolean expression
 * because Rumur thinks it is a numeric value and rejects it.
 *
 * If this bug is present, the model will fail code generation with a message
 * like "condition of if clause is not a boolean expression". If everything is
 * OK, code generation should complete successfully and the generated verifier
 * should not find any errors.
 */

const
  NOT_TRUE: false;

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  if NOT_TRUE then
    x := !x;
  end;
end;

rule begin
  x := !x;
end;
