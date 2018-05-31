type
  en: enum { A, B };

var
  x: en;

startstate begin
  x := A;
end;

rule
  var y: en;
begin
  y := A;
end;
