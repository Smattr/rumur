-- rumur_flags: smt_args()
-- skip_reason: 'no SMT solver available' if smt_args() is None else None

-- a double indirected version of smt-typeexprid.m

type
  r: 2 .. 10;
  s: r;

var
  x: s;
  y: boolean;

startstate begin
  y := true;
end;

rule begin
  if x > 1 then
    y := !y;
  end;
end;
