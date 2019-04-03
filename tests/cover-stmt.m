-- checker_output: None if xml else re.compile(r'^\s*cover "x was true" hit 1 times$', re.MULTILINE)

-- Test a cover property used as a statement

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
  cover "x was true" x;
end;
