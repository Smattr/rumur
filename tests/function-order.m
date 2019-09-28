-- rumur_exit_code: 1

-- test that calling a function that is defined later is rejected

var
  x: boolean;

function foo(n: boolean): boolean; begin
  return bar(n);
end;

function bar(n: boolean): boolean; begin
  return n;
end;

startstate begin
  x := true;
end;

rule begin
  x := !foo(x);
end;
