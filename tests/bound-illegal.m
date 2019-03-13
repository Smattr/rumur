-- rumur_flags: ['--bound', 'foobar']
-- rumur_exit_code: 1

-- test that illegal --bound options are rejected

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;
