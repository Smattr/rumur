-- Test of isundefined on various decls

var
  x: boolean;

function bar(var y: boolean): boolean; begin
  return isundefined(y);
end;

function baz(): boolean;
  var y: boolean; begin
  return isundefined(y);
end;

startstate begin
  x := true;
end;

rule bar(x) ==> begin
  x := !x;
end;

rule baz() ==> begin
  x := !x;
end;

rule
  var y: boolean; begin
  x := isundefined(y);
end;

rule begin
  x := !x;
end;
