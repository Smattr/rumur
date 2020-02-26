-- rumur_flags: ['--pack-state', 'off', '--bound', '100']

-- basic test that using an unpacked state works

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;
