var
  x: boolean;

ruleset y: boolean do
  startstate begin
    x := y;
  end;
end;

rule begin
  x := !x;
end;
