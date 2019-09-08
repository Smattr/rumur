-- checker_exit_code: 1
-- checker_output: None if xml else re.compile(r'^Rule "foo" fired', re.MULTILINE)

-- Test that smart quotes can be used to delimit a string. This test case works
-- by deliberately triggering a model error that should cause a backtrace
-- containing a correctly parsed version of the rule name.

var
  x: boolean

startstate begin
  x := true;
end

rule “foo” begin
  x := false;
end
