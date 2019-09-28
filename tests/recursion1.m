-- test a recursive function definition

var
  x: 0 .. 55;

function fib(n: 0 .. 10): 0 .. 100; begin
  if n = 0 then
    return 0;
  elsif n = 1 then
    return 1;
  else
    return fib(n - 1) + fib(n - 2);
  end;
end;

startstate begin
  x := 0;
end;

rule x <= 10 ==> begin
  x := fib(x);
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
