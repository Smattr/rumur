-- rumur_flags: ['--counterexample-trace', 'off']
-- checker_exit_code: 1

/* This model tries to provoke a bug first observed on commit
 * e96fd5201027ea9d5027637f8d42800897171d2e. If this bug has been reintroduced,
 * generating a verifier with `--counterexample-trace off` will result in C code
 * that fails to compile.
 */

var
  x: 0 .. 9;

startstate begin
  x := 0;
end;

rule begin
  x := x + 1;
end;

invariant x != 7;
