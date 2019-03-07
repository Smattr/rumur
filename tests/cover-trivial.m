-- checker_output: re.compile(r'^\s*cover 0 hit 2 times$', re.MULTILINE)

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
