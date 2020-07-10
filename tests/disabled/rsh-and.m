-- While addressing the issue described in and-return.m, it was discovered that
-- this problem also affected right shifts. If the problem still exists (or has
-- been reintroduced) for right shifts, this model will fail code generation
-- with an error like "cannot retrieve the type of an unresolved '&' expression"

var
  x: 0 .. 2;

startstate begin
  x := 1;
end;

rule x = 1 ==> begin
  x := 4 >> (x & 1);
end;

rule x = 2 ==> begin
  x := 1;
end;
