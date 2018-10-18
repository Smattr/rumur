-- rumur_flags: ['--deadlock-detection', 'stuck']
-- This model previously triggered a bug wherein Rumur would segfault during
-- code generation.

type
  en: enum { A, B };

var
  x: en;

startstate begin
  x := A;
end;

rule
  var y: en;
begin
  y := A;
end;
