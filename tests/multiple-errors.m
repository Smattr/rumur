-- checker_exit_code: 1
-- A model with multiple errors to test --max-errors option
-- TODO: tweak cmd line args to this and regex the output during testing

var
  x: 0 .. 5;

startstate begin
  x := 0;
end;

rule x < 4 ==> begin
  x := x + 2;
end;

rule x > 1 ==> begin
  x := x - 2;
end;

rule x < 5 ==> begin
  x := x + 1;
end;

rule x > 0 ==> begin
  x := x - 1;
end;

invariant x != 5;
invariant x != 4;
