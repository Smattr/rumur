-- checker_output: None if xml else re.compile(r'^\s*cover "x was 2" not hit$', re.MULTILINE)
-- checker_exit_code: 1

-- Test a set of multiple covers

var
  x: 0 .. 2;

startstate begin
  x := 0;
end;

rule begin
  x := 1 - x;
end;

cover "x was 0" x = 0;
cover "x was 1" x = 1;
cover "x was 2" x = 2;
