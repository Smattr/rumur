-- rumur_flags: smt_args()
-- skip_reason: 'no SMT solver available' if smt_args() is None else None

-- similar to smt-range.m but using a named type

type
  r: 2 .. 10;

var
  x: r;
  y: boolean;

startstate begin
  y := true;
end;

rule begin
  if x > 1 then
    y := !y;
  end;
end;
