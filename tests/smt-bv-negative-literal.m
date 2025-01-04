-- rumur_flags: smt_bv_args()
-- skip_reason: 'no SMT solver available' if smt_bv_args() is None else None

-- similar to smt-negative-literal, but for --smt-bitvectors on

const
  N: -10;

var
  x: boolean;
  y: boolean;

startstate begin
  y := true;
end;

rule begin
  if x | !x then
    y := !y;
  end;
end;
