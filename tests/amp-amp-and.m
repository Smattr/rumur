-- test that Rumur extension && is accepted

var
  x: boolean

startstate begin
  x := true;
end

rule begin
  x := !(x && x);
end
