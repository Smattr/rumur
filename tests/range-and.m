-- While addressing the issue described in and-return.m, it was observed that
-- ranges were not handled correctly with respect to disambiguation either. If
-- this problem still exists (or has been reintroduced) this model will
-- incorrectly fail code generation with an error like "cannot constant fold an
-- unresolved '&' expression".

type
  t: 0 .. 1 & 1;

var
  x: t;

startstate begin
  x := 1;
end;

rule begin
  x := 1 - x;
end;
