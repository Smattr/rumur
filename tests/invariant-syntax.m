-- A model that tests invariants with flipped syntax

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;

invariant "hello" x | !x;
invariant x | !x "world";
