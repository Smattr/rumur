/* This model is intended to provoke an issue that was observed on commit
 * 7eafb5295a1f2f094579ef259b537bd3e4996158. If the issue has been reintroduced,
 * the generated verifier for this model will cause a warning when compiled
 * about passing a const object as a non-const parameter.
 *
 * The cause of this was that the state is always passed in as the first
 * parameter to a model function. Specifically, it is passed as a non-const
 * parameter. Meanwhile, the state variable that is available in the context of
 * a rule guard is const. Obviously the function "ok" is safe to call in the
 * guard below because it does not modify any state variables.
 *
 * A compiler warning may not seem like a particularly significant issue, but
 * one of the secondary goals of Rumur is for the generated verifier to always
 * compile warning-free. The advantage of this is that you can build your
 * generated verifier with "-W -Wall -Wextra" and any warnings you see are
 * likely to indicate actual problems in your input model.
 */
var
  x: boolean;

function ok(): boolean; begin
  return true;
end;

startstate begin
  x := true;
end;

rule ok() ==> begin
  x := !x;
end;
