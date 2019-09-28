-- test a recursive procedure definition

var
  x: 0 .. 55;

procedure set(n: 0 .. 10); begin
  if n = 0 then
    x := 0;
  elsif n = 1 then
    x := 1;
  else
    set(n - 2);
  end;
end;

startstate begin
  x := 0;
end;

rule x <= 10 ==> begin
  set(x);
end;

rule begin
  x := 2;
end;

rule begin
  x := 5;
end;

rule begin
  x := 10;
end;
