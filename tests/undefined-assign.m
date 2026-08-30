-- can we handle the 'undefined' token in an assignment?
--
-- This seems to be an extension added to CMurphi after its initial release.

var
  x: boolean;

startstate begin
  x := false;
end;

rule
  var y: boolean;
begin
  y := x;
  x := undefined;
  assert isundefined(x);
  x := !y;
end;
