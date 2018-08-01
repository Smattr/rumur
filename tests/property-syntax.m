-- Test the "property" keyword

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;

-- This property never gets enabled, so its violation should never surface as an
-- error.
property "hello world" x;
