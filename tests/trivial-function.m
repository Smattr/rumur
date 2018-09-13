var x: boolean;

procedure foo(); begin
end;

function bar(): boolean; begin
  return true;
end;

startstate begin
  x := true;
end;

rule begin
  x := bar();
end;

rule begin
  foo();
end;

rule begin
  x := !x;
end;
