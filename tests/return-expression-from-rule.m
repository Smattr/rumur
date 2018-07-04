-- rumur_exit_code: 1

-- Testing return with an expression within a rule. This is illegal and should
-- fail.

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := true;
  return 3;
  x := false;
end;

invariant x;
