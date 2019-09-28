/* This model contains an extraneous semi-colon. On commit
 * 90a95ea10ef3eeb15803954c0f8c9fe267799b94, it was observed that Rumur
 * incorrectly rejects this as a syntax error. This tests whether the problem
 * has been reintroduced.
 */

var
  x: boolean;

startstate begin
  x := false;
end;

rule begin
  x := !x;;
end;
