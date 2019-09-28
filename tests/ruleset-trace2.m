-- checker_exit_code: 1
-- checker_output: None if xml else re.compile(r'^Rule\s+2\s*,\s*y\s*:\s*1\s*,\s*z\s*:\s*8\s*,\s*w\s*:\s*-32\s+fired\b', re.MULTILINE)

/* Similar to ruleset-trace.m, but with a benign first ruleset that ensures the
 * ruleset we're expecting to appear in the trace does not end up being index 1.
 * This is relevant with respect to how Rumur works internally, numbering rule
 * transitions via the rule_taken variable.
 */

type
  t1: 0 .. 1;
  t2: 4 .. 10;
  t3: -44 .. -2

var
  x: boolean;

startstate begin
  x := true;
end;

-- an innocuous rule to make sure our problematic ruleset below will not get a
-- trivial index
ruleset y: t1; z: t2 do
  rule begin
    x := !x;
  end;
end;

ruleset y: t1; z: t2; w: t3 do

  rule begin
    assert y != 1 | z != 8 | w != -32 "failed assertion";
    x := !x;
  end;

end;

