/* A more generalised version of error-string-injection.m, where we try printf
 * format codes in several potentially vulnerable locations.
 */

var
  x: boolean

startstate "hello %s world 1" begin
  x := true;
end

rule "hello %s world 2" begin
  if false then
    error "hello %s world 3";
  end;
  assert "hello %s world 4" x | !x;
  x := !x;
end

assume "hello %s world 5" x | x;

cover "hello %s world 6" x | x;
