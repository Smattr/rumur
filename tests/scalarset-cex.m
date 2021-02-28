-- checker_exit_code: 1
-- checker_output: None if xml or multithreaded else re.compile(r'\ba\b.*?\bz:\s*t_6\b(.|\n)*?\bx\[t_6]:\s*true\b(.|\n)*?\bc\b.*?\bz:\s*t_6\b(.|\n)*?\by:\s*t_6', re.MULTILINE)

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
