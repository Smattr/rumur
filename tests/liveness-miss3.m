-- checker_exit_code: 1
-- checker_output: None if xml else re.compile(r'liveness property "x is 7" violated')

-- a variant of liveness-miss1.m that covers all values but where 7 is not live

var
  x: 0 .. 10

startstate begin
  x := 0;
end

rule x < 10 ==> begin
  x := x + 1;
end

rule x > 0 & x != 8 ==> begin
  x := x - 1;
end

liveness "x is 7" x = 7
