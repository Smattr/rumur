var
  x: 0 .. 10;

procedure foo(y: 0 .. 5); begin
end;

startstate begin
  x := 0;
end;

rule begin
  foo(x);
  x := 1 - x;
end;
