-- rumur_flags: ['--smt-simplification', 'on', '--smt-path', 'python', '--smt-arg', os.path.join(os.path.dirname(__file__), '../misc/toy-smt.py')]

/* This model tests whether Rumur is capable of simplifying simple conditions at
 * code generation time. If it is, then it will replace the `y = y` check with
 * true and this model will pass. If not, the check will remain and cause a read
 * of an undefined value at runtime.
 */

var
  x: boolean;
  y: boolean;

startstate begin
  x := true;
end;

rule begin
  if y = y then
    x := !x;
  end;
end;
