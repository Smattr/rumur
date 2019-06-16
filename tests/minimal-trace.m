-- checker_exit_code: 1
-- checker_output: None if xml else re.compile(r'^\s*invariant "path 1" failed', re.MULTILINE)

/* This model is designed to provoke a longer-than-minimal error trace. If the
 * --shortest-trace option is functioning correctly, Rumur should correctly
 * suppress this longer trace and instead find the shorter but longer running
 * trace.
 */

var
  x: 0 .. 100;
  z: 0 .. 100;
  w: 0 .. 100;
  path1: boolean;
  flag: boolean;
  path2: boolean;

startstate begin
  x := 0;
  z := 0;
  w := 0;
  path1 := false;
  path2 := false;
  flag := false;
end;

rule x < 100 ==> begin
  x := x + 1;
end;

rule x > 0 ==> begin
  x := x - 1;
end;

rule z < 100 ==> begin
  z := z + 1;
end;

rule z > 0 ==> begin
  z := z - 1;
end;

rule w < 100 ==> begin
  w := w + 1;
end;

rule w > 0 ==> begin
  w := w - 1;
end;

rule z > 90 | w > 90 ==> begin
  flag := true;
end;

rule flag ==> begin
  path2 := true;
end;
  
rule x > 90 ==> begin
  for y: 0 .. 1000000 do
    x := x;
  end;
  path1 := true;
end;

invariant "path 1" !path1;
invariant "path 2" !path2;
