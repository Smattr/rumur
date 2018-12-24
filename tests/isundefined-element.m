-- Test of isundefined on nested values.

var
  x: array [0 .. 0] of boolean;
  y: record
    z: boolean;
  end;

startstate begin
  x[0] := true;
  y.z := true;
end;

rule begin
  x[0] := isundefined(x[0]);
end;

rule begin
  y.z := isundefined(y.z);
end;

rule begin
  x[0] := !x[0];
end;
