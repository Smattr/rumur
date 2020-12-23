-- This test is designed to provoke a case where murphi2c would generate
-- malformed code. If this issue has been reintroduced, murphi2c will produce
-- code that causes a malformed expansion of the `x` macro. See also Github
-- issue #207.

type
  t: record
    x: boolean;
  end;

var
  y: t;

startstate begin
  y.x := true;
end;

rule begin
  alias x: y.x do
    x := !y.x;
  end;
end;
