-- This test case attempts to provoke an edge case in murphi2c. '\' is an escape
-- marker in C comments, but is not significant in Murphi comments. A naive
-- translation to C which preserves comments will emit a malformed comment with
-- an escape that swallows the next line of code.

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  -- single line comment that ends in \
  if true then
    x := !x;
  end;

  -- a similar case that also has white space after the \ 
  if false then
    x := !x;
  end;
end;
