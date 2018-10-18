-- rumur_flags: ['--deadlock-detection', 'stuck']

-- Testing return statements within startstates.

var
  x: boolean;

startstate begin
  x := true;
  return;
  x := false;
end;

rule begin
  x := true;
end;

invariant x;
