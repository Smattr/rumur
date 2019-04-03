-- checker_output: re.compile(r'Rule &quot;foo&quot;') if xml else None
-- checker_exit_code: 1

/* This tests for the presence of a bug first observed on commit
 * d3517bbf0c443242704979a4684777a9f1aeddfa. Rule names and other strings dealt
 * with by xml_escape() in the verifier were treated incorrectly. For characters
 * that do not need escaping, the output pointer is not incremented, resulting
 * in them being omitted. If the bug has been re-introduced, you will not see
 * Rule foo indicated in the counter-example trace.
 */

var
  x: boolean;

startstate begin
  x := true;
end;

rule "foo" begin
  x := !x;
end;

invariant x;
