-- This model tests a case of recursion where the recursive function has a prior
-- call to a non-recursive function. This is interesting because at time of
-- writing (commit 084489251754d9ccd9a23d7b3fba0989d338eb80) the recursive call
-- to bar will be resolved to an incomplete, unresolved definition of bar. I.e.
-- a version of bar in which the call to foo still has an unresolved target. It
-- is suspected that this might be an error where a subtle bug could be provoked
-- in future, so this test was introduced to guard against that.

var
  x: boolean;

function foo(x: boolean): boolean; begin
  return !x;
end;

function bar(x: boolean): boolean; begin
  foo(x);
  if x then
    bar(!x);
  end;
  return !x;
end;

startstate begin
  x := true;
end;

rule begin
  x := bar(x);
end;
