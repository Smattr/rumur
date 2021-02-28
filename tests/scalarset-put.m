-- checker_output: None if xml else re.compile(r'^y:\s*Undefined$(.|\n)*?^y:\s*t_9$', re.MULTILINE)

-- Test that we can print a scalarset variable. The initial implementation of
-- scalarset remapping caused code to be emitted in an order that had put
-- statements calling a function that had not yet been defined. This caused a
-- compilation failure of the generated verifier. If this problem has been
-- re-introduced, the code generated from this model will fail to compile.

type
  t: scalarset(10);
var
  x: boolean;
  y: t;

startstate begin
  -- print it while it is undefined
  put y;

  -- assign it a value
  for z: t do y := z; end;

  -- print it now that it has a value
  put y;

  x := true;
end;

rule begin
  -- print it during a rule
  put y;

  x := !x;
end;
