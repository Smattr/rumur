-- test of using liveness inside a ruleset block

var
  x: 0 .. 10

startstate begin
  x := 0;
end

rule x < 10 ==> begin
  x := x + 1;
end

rule x > 0 ==> begin
  x := x - 1;
end

ruleset y: 0 .. 10 do
  liveness "x is y" x = y
end
