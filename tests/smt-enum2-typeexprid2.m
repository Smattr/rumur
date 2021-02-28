-- rumur_flags: CONFIG['SMT_ARGS']
-- skip_reason: 'no SMT solver available' if CONFIG['SMT_ARGS'] is None else None

-- a double indirected version of smt-enum2-typeexprid.m

type
  t: enum { A, B };
  t2: t;

var
  x: t2;
  y: boolean;

startstate begin
  y := true;
end;

rule begin
  -- if the SMT bridge can deal with enums, it should turn the following
  -- condition into `true`, avoiding a read of an undefined value from x
  if x = A | x = B then
    y := !y;
  end;
end;
