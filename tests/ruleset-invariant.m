-- A model involving an invariant nested in a ruleset. This is relatively rare
-- construct, but is compliant with the grammar.

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;

ruleset y: boolean do
  invariant x = y | x = !y;
end;
