-- checker_exit_code: 1
-- checker_output: None if xml else re.compile(r'liveness property "x is y" violated')

-- variant of liveness-in-ruleset.m designed to fail

var
  x: 0 .. 10

startstate begin
  x := 0;
end

rule x < 9 ==> begin
  x := x + 1;
end

rule x > 0 ==> begin
  x := x - 1;
end

ruleset y: 0 .. 10 do
  liveness "x is y" x = y
end
