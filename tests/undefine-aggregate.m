-- rumur_flags: ['--deadlock-detection', 'stuck']

-- test that we can 'undefine' an aggregate
var
  x: record
    a: boolean;
    b: boolean;
  end;
  y: boolean;

startstate begin
  x.a := true;
  x.b := false;
  y := true;
end;

rule y ==> begin
  x.a := !x.a;
  x.b := !x.b;
end;

rule begin
  undefine x;
  y := false;
end;
