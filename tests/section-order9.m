-- rumur_exit_code: 1

-- test that functions within rulesets are rejected

var
  x: boolean;

startstate begin
  x := true;
end;

ruleset y: boolean do

  function foo(): boolean; begin
    return true;
  end;

  rule begin
    x := y;
  end;

end;
