-- Test that duplicate rule names are accepted. Note that we define the two
-- rules such that omitting either will cause a deadlock during checking.

var
  x: boolean;

startstate begin
  x := true;
end;

rule "foo" begin
  x := true;
end;

rule "foo" begin
  x := false;
end;
