-- skip_reason: 'N/A in XML mode' if xml else 'N/A in non-debug mode' if not debug else None
-- checker_output: re.compile(r'\bfield x is located at state offset 2 bits$(.|\n)*\bfield y is located at state offset 0 bits$', re.MULTILINE)

-- This tests for a regression of a problem first observed on commit
-- 4944427734628cf913e8d5eeb54d897033f9eb59. When state variables should have
-- been reordered during optimisation, they were not. If this bug has been
-- reintroduced, the debug output will incorrectly list:
--
--   * field y is located at state offset 30 bits
--   * field x is located at state offset 0 bits

type
  t: scalarset(10);

var
  -- this field has a width of 30 bits in generated code and should get
  -- automatically reordered to come after the second field...
  x: array[t] of 0 .. 5;

  -- ...that has a width of 2 bits — a power of two — and hence should come
  -- first
  y: boolean;

startstate begin
  y := true;
end;

ruleset z: t do
  rule begin
    x[z] := 5;
  end;
end;

rule begin
  y := !y;
end;
