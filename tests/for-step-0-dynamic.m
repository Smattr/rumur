-- checker_exit_code: 1

-- this model tests that a step in a for loop that is not a generation-time
-- constant but ends up being 0 at runtime is caught during checking

var
  y: boolean;

startstate begin
  y := true;
end;

rule
  var x: 0 .. 10;
begin
  x := 0;
  for i := 0 to 2 by x do
    put "hello world";
  end;
  y := !y;
end;
