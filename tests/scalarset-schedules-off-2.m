-- checker_exit_code: 1
-- rumur_flags: ['--scalarset-schedules', 'off']
-- checker_output: None if xml or multithreaded else re.compile(r'^((?!t_)(.|\n))*$')

-- A variant of scalarset-schedules-off.m that checks startstate parameters are
-- also not printed symbolically.

type
  t: scalarset(10);

var
  x: array[t] of boolean;
  y: t;

ruleset w: t do
  startstate begin
    for z: t do x[z] := false; end;
  end;
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
