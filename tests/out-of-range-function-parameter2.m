-- checker_exit_code: 1

/* On 466fe5b288aaa45c9970e1391701dd8653f8ab82 it was observed that the
 * following model results in generated code that causes a compiler warning when
 * compiling due to unsigned/signed comparison.
 */

var
  x: boolean

procedure foo(y: -65537 .. -2); begin
  if y = 0 then
  end;
end;

startstate begin
  x := true;
end;

rule
  var z: 65537 .. 65537;
begin
  z := 65537;
  foo(z);
  x := !x;
end;
