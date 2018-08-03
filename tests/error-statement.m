-- checker_exit_code: 1
-- Test a basic error statement.
var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
  error "hello world";
end;
