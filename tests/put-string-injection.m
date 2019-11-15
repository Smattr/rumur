/* This model is designed to trigger a problem first observed on
 * 0f66cb1de6e4eb359fc4aa1ab4d55c8dd951a04d wherein a crafted call to put could
 * result in the generation of code that would corrupt its own stack. If this
 * problem has been re-introduced, the generated code for this model will
 * trigger a compilation error when passing -Werror=format.
 */

var
  s: 0 .. 10;
  a: array[0 .. 10] of boolean;

startstate begin
  a[0] := true;
  s := 1;
end;

rule begin
  put a[4%s];
  a[0] := !a[0];
end;
