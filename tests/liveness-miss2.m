-- checker_exit_code: 1
-- checker_output: None if xml else re.compile(r'liveness property "x is 10" violated')

-- a variant of liveness-miss1.m that leaves two values unreached

var
  x: 0 .. 10

startstate begin
  x := 0;
end

rule x < 8 ==> begin
  x := x + 1;
end

rule x > 0 ==> begin
  x := x - 1;
end

liveness "x is 10" x = 10
