-- a variant of alias-of-alias-rule.m that uses multiple aliasrules, instead of
-- one

var
  x: boolean;

startstate begin
  x := true;
end;

alias y: x do
  alias z: y do
    rule begin
      z := !z;
    end;
  end;
end;
