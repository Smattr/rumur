-- rumur_flags: ['--deadlock-detection', 'stuck']

-- Testing return statements within rules within rulesets.

var
  x: boolean;

startstate begin
  x := true;
end;

ruleset y: boolean do
  rule begin
    x := true;
    return;
    x := false;
  end;
end;

invariant x;
