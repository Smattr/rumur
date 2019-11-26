/* This model checks we have not reintroduced a problem first observed on commit
 * e1b74738ba73df5352622464be47b69db688c633, wherein invariant names involving
 * printf format codes would cause bad calls to the function error() to be
 * generated in the verifier. If this problem has been introduced, the generated
 * model will fail to compile with the given flags.
 */

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;

invariant "test string %s injection" x | !x;
