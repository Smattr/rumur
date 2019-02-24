-- checker_exit_code: 1

-- Test reading of an undefined record value.

var
  x: record
    a: boolean;
  end;

startstate begin
end;

rule begin
  x.a := !x.a;
end;
