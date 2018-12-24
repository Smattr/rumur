-- Basic test of isundefined

var
  x: boolean;

startstate begin
  if isundefined(x) then
    x := true;
  else
    x := false;
  end;
end;

rule isundefined(x) ==> begin
  x := true;
end;

rule !isundefined(x) ==> begin
  x := !x;
end;

rule begin
  undefine x;
end;
