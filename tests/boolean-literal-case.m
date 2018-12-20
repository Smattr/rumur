/* A bug observed in commit f6c5ca123d58d1620ab9faaa804e03e075b6ca62 was that
 * boolean literals that were not lower case were not understood by Rumur, and
 * resulted in a syntax error. This model tests that this problem has not been
 * reintroduced.
 */

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  if TRUE then end;
  if TrUe then end;
  if True then end;
  if FALSE then end;
  if fAlSe then end;
  if False then end;
  x := !x;
end;
