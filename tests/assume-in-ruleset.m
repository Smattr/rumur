-- Test using an assumption inside a ruleset

var
  x: 0 .. 10

startstate begin
  x := 0;
end

rule x > 0 ==> begin
  x := x - 1;
end

rule x < 4 ==> begin
  x := x + 1;
end

ruleset y: 5 .. 10 do
  assume x != y
end
