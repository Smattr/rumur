type
  foo_t: 1 .. 10;

var
  x: boolean;

startstate begin
  x := true;
end;

ruleset y: foo_t do
  rule begin
    x := !x;
  end;
end;
