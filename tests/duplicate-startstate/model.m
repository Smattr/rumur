-- In earlier incarnations of Rumur, models like this would trigger a memory
-- leak as the duplicate startstate that was encountered would not be freed.

var
  x: boolean;

startstate begin
  x := true;
end;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;
