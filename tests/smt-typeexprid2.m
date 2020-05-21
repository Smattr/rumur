-- rumur_flags: self.config['SMT_ARGS']
-- skip_reason: 'no SMT solver available' if self.config['SMT_ARGS'] is None else None

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
