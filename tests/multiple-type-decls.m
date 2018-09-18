-- Test declaration of multiple types at once.

type
  A, B: record x: boolean; end;

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;

