-- rumur_flags: smt_args()
-- skip_reason: 'no SMT solver available' if len(smt_args()) == 0 else None

/* test that the SMT bridge is capable of dealing with formulas involving
 * integer constants
 */

const
  C: 2;

var
  x: 2 .. 2;
  y: boolean;

startstate begin
  y := true;
end;

rule begin
  -- if the SMT bridge can deal with constants, it should judge this condition
  -- to be tautological and turn it into just `true`, avoiding an error from
  -- reading x while it is undefined
  if x = C then
    y := !y
  end;
end;
