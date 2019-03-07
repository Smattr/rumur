-- checker_output: re.compile(r'^\s*cover 0 hit 2 times$', re.MULTILINE)

-- Test of a cover property that is hit multiple times

var
  x: boolean;
  y: boolean;

startstate begin
  x := true;
  y := true;
end;

rule begin
  x := !x;
end;

rule begin
  y := !y;
end;

cover "x was false" !x;
