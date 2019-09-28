-- checker_exit_code: 1

-- Test indexing into an array out of bounds.

var
  x: array[0 .. 1] of boolean;

startstate begin
end;

rule begin
  x[3] := false;
end;
