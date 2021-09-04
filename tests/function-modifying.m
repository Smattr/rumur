-- call to a function that also takes a var parameter
--
-- While developing murphi2uclid, it emerged that an interesting case was a call
-- to a value-returning function where the function itself also took a var
-- parameter. Examples of this existed elsewhere in the test suite, but only in
-- combination with other features not accepted by murph2uclid. This example is
-- intended to be acceptable by murphi2uclid, but still exhibit this described
-- behaviour.

var
  x: boolean;
  z: boolean;

function foo(var y: boolean): boolean; begin
  -- we cannot actually modify y and still have this model acceptable to
  -- murphi2uclid (yes, this is a pretty weird edge case)
  return !y;
end;

startstate begin
  x := true;
  z := true;
end;

rule begin
  -- call the function and store its return value, but also pass it something
  -- mutable
  z := foo(x);
  x := z;
end;
