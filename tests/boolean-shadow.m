-- rumur_exit_code: 1

-- test that it is not possible to shadow the 'boolean' type

var
  x: boolean;
  y: 0 .. 1;

startstate begin
  x := true;
end;

rule
  type boolean: 0 .. 1;
  var z: boolean;
begin
  z := 0;
  y := z;
  x := !x;
end;
