-- rumur_exit_code: 1

/* This model incorrectly uses a call to a procedure within an expression, as if
 * the procedure returned a number. In commit
 * 366762cc01cfbb09193ec23ba9762260b458b016, this triggers a bug where Rumur
 * thinks this is a valid model and generates a verifier that does not compile.
 */

var
  x: 0 .. 10;

procedure foo(); begin
end;

startstate begin
  x := 1;
end;

rule begin
  x := 10 - foo();
end;
