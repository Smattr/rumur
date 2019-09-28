-- checker_exit_code: 1

-- Test writing a value out of range into a record.

var
  x: record
    a: 0..1;
  end;

startstate begin
  x.a := 0;
end;

rule begin
  x.a := x.a + 1;
end;
