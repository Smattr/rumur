-- This model tests for a bug first observed in commit
-- be143484d0b7d00c1cda9d9f7c5b4a2045eabe2b wherein murphi2c would produce C
-- source code that would fail to compile. If the bug has been re-introduced,
-- the conditional expression will be emitted in the C file missing surrounding
-- brackets.

var
  x: boolean;

function foo(): boolean; begin
  return true;
end;

startstate begin
  x := false;
end;

rule begin
  if foo() then
    x := !x;
  end;
end;
