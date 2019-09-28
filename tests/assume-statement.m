-- test using an assumption as a statement

var
  x: 0 .. 2

startstate begin
  x := 0;
end

rule begin
  x := 1 - x;
  assume x != 2;
end
