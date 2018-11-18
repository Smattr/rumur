-- rumur_exit_code: 1

-- A test that we can correctly escape characters in strings.

var
  x: boolean;

startstate begin
  x := true;
end;

rule "hello\" begin
  x := !x;
end;
