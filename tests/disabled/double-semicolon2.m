-- similar to double-semicolon.m, but using doubles in other places

var
  x: boolean; ;

startstate begin
  x := false;
end;;;

rule begin
  x := !x;;
end; ;	;
