-- rumur_flags: smt_bv_args()
-- skip_reason: 'no SMT solver available' if smt_bv_args() is None else None

-- test that the SMT bridge can simplify bitwise AND when using bitvector logic

var
  x: 1 .. 2
  y: boolean

startstate begin
  y := true;
end

rule begin
  -- if the SMT bridge can handle bitwise AND, it should simplify the following
  -- to `true`, avoiding an undefined read
  if (x & 1) = 1 | x = 2 then
    y := !y;
  end;
end
