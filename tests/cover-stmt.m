-- checker_output: re.compile(r'^\s*cover 0 hit 1 times$', re.MULTILINE)

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
