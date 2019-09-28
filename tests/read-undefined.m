-- checker_exit_code: 1

-- Test reading of an undefined value.

var
  x: boolean;

startstate begin
end;

rule begin
  x := !x;
end;
