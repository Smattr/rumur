-- rumur_flags: ['--counterexample-trace', 'off']
-- checker_exit_code: 1

-- This tests deliberately reading an undefined value from a scalarset-indexed
-- array. While this should pretty obviously fail, when an error message is
-- printed about the failure it needs to describe the failing scenario. If we
-- have implemented this incorrectly, it may try to access the 'schedule' (which
-- permutation of a scalarset is in use) which is unavailable when
-- counterexample traces are off.

type
  t: scalarset(10);

var
  y: array[t] of boolean;

startstate begin
end;

ruleset w: t do
  rule begin
    y[w] := !y[w];
  end;
end;
