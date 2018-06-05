type
  foo_t: 0 .. 1073741824;

var
  x: foo_t;

startstate begin
  x := 0;
end;

rule -- expand this limit in future?
  x < 10000000 ==>
begin
  x := x + 1;
end;

rule
  x > 0 ==>
begin
  x := x - 1;
end;
