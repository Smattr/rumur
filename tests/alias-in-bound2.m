-- variant on alias-in-bound.m that uses the alias in a local range instead of a
-- ruleset range

const
  N: 2;

var
  x: 0 .. N;

startstate begin
  x := 0;
end;

rule begin
  alias y: N do
    for i: 0 .. y do
      x := N - i;
    end;
  end;
end;

rule begin
  x := N - x;
end;
