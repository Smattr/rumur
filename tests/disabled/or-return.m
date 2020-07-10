-- equivalent of and-return.m but with an OR

var
  x: boolean;

function foo(): boolean; begin
  return x | x;
end;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;
