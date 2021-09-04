-- checker_exit_code: 1
-- checker_output: None if xml else re.compile(r'^Startstate\s+1\s*,\s*y\s*:\s*1\s*,\s*z\s*:\s*8\s*,\s*w\s*:\s*-32\s+fired\b', re.MULTILINE)

/* This is a similar test to ruleset-trace.m but using a ruleset-contained
 * startstate, instead of an ordinary rule.
 */

type
  t1: 0 .. 1;
  t2: 4 .. 10;
  t3: -44 .. -2

var
  x: boolean;

ruleset y: t1; z: t2; w: t3 do
  startstate begin
    assert y != 1 | z != 8 | w != -32 "failed assertion";
    x := true;
  end;
end;

rule begin
  x := !x;
end;
