-- rumur_flags: CONFIG['SMT_ARGS']
-- skip_reason: 'no SMT solver available' if CONFIG['SMT_ARGS'] is None else None

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
