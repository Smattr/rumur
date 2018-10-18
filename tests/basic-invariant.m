-- rumur_flags: ['--deadlock-detection', 'stuck']

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := x;
end;

invariant x;
