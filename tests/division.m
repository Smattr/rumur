-- If you can believe it, my initial implementation left out the division
-- operator. We have this test case just to make sure it works.

var
  x: 0 .. 2;

startstate begin
  x := 2;
end;

rule begin
  x := x / 2;
end;

rule x < 2 ==> begin
  x := x + 1;
end;
