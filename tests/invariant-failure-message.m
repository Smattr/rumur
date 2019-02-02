-- checker_exit_code: 1
-- checker_output: re.compile(r'^.*invariant "hello world" failed', re.MULTILINE)

/* Test that the checker reports the name of an invariant in its failure
 * message.
 */

var
  x: boolean

startstate begin
  x := true;
end

rule begin
  x := !x;
end

invariant "hello world" false
