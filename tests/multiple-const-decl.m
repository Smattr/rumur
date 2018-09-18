-- Test declaration of multiple constants at once.

const
  A, B: 10;

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;
