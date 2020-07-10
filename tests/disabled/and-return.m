-- This model tests for regression of a bug reported against Rumur v2020.05.18.
-- Following the introduction of bitwise operations, like bitwise AND, the token
-- '&' is initially parsed as an 'AmbiguousAmp'. A later pass attempts to
-- discriminate this into a bitwise AND or a logical AND. However, seemingly
-- this did not happen before function return value checking. If this bug has
-- been reintroduced, this model will fail code generation with an error like
-- "cannot retrieve the type of an unresolved '&' expression".

var
  x: boolean;

function foo(): boolean; begin
  -- the following line is the tricky one to handle
  return x & x;
end;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;
