-- checker_exit_code: 1
-- rumur_flags: ['--scalarset-schedules', 'off']
-- checker_output: None if xml or multithreaded else re.compile(r'^((?!t_)(.|\n))*$')

-- A variant of scalarset-cex.m to check that switching off scalarset schedules
-- still produces a verifier that compiles and runs. The regex we use above
-- demands that the symbolic value of a scalarset, e.g. "t_3", never appears in
-- the output.

type
  t: scalarset(10);

var
  x: array[t] of boolean;
  y: t;

startstate begin
  for z: t do x[z] := false; end;
end;

ruleset z: t do
  rule "a" begin
    x[z] := true;
  end;

  rule "b" begin
    x[z] := false;
  end;

  rule "c" begin
    y := z;
  end;
end;

invariant
  forall z: t do isundefined(y) | isundefined(x[z]) | y != z | !x[z] end;
