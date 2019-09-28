-- checker_output: None if xml else re.compile(r'^\s*cover "dummy" hit 2 times$', re.MULTILINE)

-- Test a degenerate cover property

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;

cover "dummy" true;
