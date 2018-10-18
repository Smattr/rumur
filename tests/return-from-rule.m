-- rumur_flags: ['--deadlock-detection', 'stuck']

-- Testing return statements within rules.

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := true;
  return;
  x := false;
end;

invariant x;
