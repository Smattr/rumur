/* This model is based on ../long-running/model.m but with some extra rules
 * thrown in to generate a non-trivial queue. The utility of this is testing a
 * multithreaded checker.
 */

type
  foo_t: 0 .. 1073741824;

var
  x: foo_t;

startstate begin
  x := 0;
end;

rule
  x < 10000000 ==>
begin
  x := x + 1;
end;

rule
  x < 9999995 ==>
begin
  x := x + 5;
end;

rule
  x < 9999990 ==>
begin
  x := x + 10;
end;

rule
  x < 9999980 ==>
begin
  x := x + 20;
end;

rule
  x < 9999950 ==>
begin
  x := x + 50;
end;

rule
  x > 0 ==>
begin
  x := x - 1;
end;
