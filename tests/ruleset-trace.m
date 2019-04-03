-- checker_exit_code: 1
-- checker_output: None if xml else re.compile(r'^Rule\s+1\s*,\s*y\s*:\s*1\s*,\s*z\s*:\s*8\s*,\s*w\s*:\s*-32\s+fired\b', re.MULTILINE)

/* As of v2019.01.12, Rumur printed counterexample traces involving rulesets
 * without showing the values of the parameters to the ruleset rules. This made
 * debugging such traces virtually impossible. This tests that we correctly
 * print a trace showing the values of such parameters.
 *
 * Github: issue #105 "missing parameters in ruleset trace"
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

ruleset y: t1; z: t2; w: t3 do

  rule begin
    assert y != 1 | z != 8 | w != -32 "failed assertion";
    x := !x;
  end;

end;
