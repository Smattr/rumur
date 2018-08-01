-- Test assertions with both orderings of its arguments

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  assert x | !x "hello";
  x := !x;
end;

rule begin
  assert "world" x | !x;
  x := !x;
end;
