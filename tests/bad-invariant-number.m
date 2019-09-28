-- checker_exit_code: 1
-- checker_output: None if xml else re.compile(r'\binvariant 1 failed\b')

/* This test checks for the presence of a previous bug first observed in commit
 * e1b74738ba73df5352622464be47b69db688c633, wherein invariants are incorrectly
 * numbered. This problem occurs because all properties are counted together, so
 * non-invariant properties that precede an invariant cause it to get a higher
 * number than expected. If this problem has been reintroduced, the model will
 * call the invariant "invariant 3" in its error message.
 */

var
  x: boolean

startstate begin
  x := true;
end

rule begin
  x := !x;
end

rule begin
  x := !x;
end

assume true
assume true

invariant x
