-- checker_exit_code: 1

var
  x: boolean;

startstate begin
  x := true;
end;

rule x ==> begin
  x := false;
end;
