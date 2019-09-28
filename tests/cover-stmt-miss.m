-- checker_output: None if xml else re.compile(r'^\s*cover "x was 2" not hit$', re.MULTILINE)
-- checker_exit_code: 1

-- Test a cover statement that is never hit

var
  x: 0 .. 2;

startstate begin
  x := 0;
end;

rule begin
  x := 1 - x;
  cover "x was 2" x = 2;
end;

