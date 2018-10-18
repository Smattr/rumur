-- rumur_flags: ['--deadlock-detection', 'stuck']

-- There's no utility to the scalarset in this model. It's purely to validate
-- that a scalarset can exist.

type
  t: scalarset(2);

var
  x: t;

ruleset i: t do
  startstate begin
    x := i;
  end;
end;

rule begin
  x := x;
end;
