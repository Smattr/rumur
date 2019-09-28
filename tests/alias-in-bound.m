-- this model provokes a problem first observed on commit
-- 538dd85228b98fce4e506883523c0afbf9647e0f wherein an alias of a constant would
-- not be recognised as a constant as thus not be usable in a range

const
  N: 2;

var
  x: 0 .. N;

startstate begin
  x := 0;
end;

alias y: N do
  ruleset i: 0 .. y do
    rule begin
      x := N - i;
    end;
  end;
end;
