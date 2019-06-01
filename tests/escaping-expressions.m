-- checker_exit_code: 1

/* This model intentionally provokes an out-of-range indexing error, but with an
 * expression that contains a character that is sensitive in XML. When using the
 * machine-readable output format on commit
 * 5ea35510cacf551929203e947922ad5811a639d1 this produced invalid XML. This
 * tests whether the bug involved has been reintroduced.
 */

var
  x: array[0 .. 1] of boolean;

startstate begin
  x[0] := true;
  x[1] := false;
end;

rule begin
  x[3 < 2 ? 3 : 2] := !x[0];
end;
