-- Test the alternative form of the assignment operator

var
  x: boolean;

startstate begin
  x ≔ true;
end;

rule begin
  x ≔ !x;
end;
