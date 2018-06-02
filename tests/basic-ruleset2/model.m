var
  x: boolean;

startstate begin
  x := true;
end;

ruleset y: boolean do
  rule begin
    x := y;
  end;
end;
