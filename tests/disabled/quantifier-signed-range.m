-- rumur_flags: ['--deadlock-detection', 'off']
-- checker_output: None if xml else re.compile(r'\b3 states\b')

/* This model tests that quantifiers that pass over zero (i.e. negative start
 * and positive end) function correctly. In
 * 99529844092fcbe1bbbfb3170c7b9a8364a6d055, a bug was observed that caused
 * loops generated from such quantifiers to iterate 0 times. This originated
 * from a change in 79579fd5ee7cc3c120439b5d3187a09ffd5dcd6e. This model checks
 * that such a problem has not been reintroduced.
 */

var
  x: -1 .. 1;

startstate begin
  x := 0;
end;

ruleset y: -1 .. 1 do
  rule begin
    x := y;
  end;
end;
