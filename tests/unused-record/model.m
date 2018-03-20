type
  foo_t: record
    x: boolean;
  end;

var
  y: boolean;

startstate begin
  y := false;
end;

rule begin
  y := !y;
end;
