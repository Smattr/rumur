-- checker_output: None if self.xml else re.compile(r'z\[6\]:false')

-- Test put statements get parsed correctly. The expected output is just chosen
-- to match one of the array entries that is correlated with mis-implementing
-- this feature. E.g. a first attempt at implementing iteration for array
-- printing made the mistake of inaccurate indexing resulting in "z[6]:true".

var
  x: boolean;
  y: record
    a: boolean;
    b: boolean;
  end;
  z: array [-4.. 7] of boolean;

startstate begin

  put "printing an uninitialised boolean...\n";
  put x;
  put "\n";

  x := false;
  put "printing an initialised boolean...\n";
  put x;
  put "\n";

  put "printing an uninitialised record...\n";
  put y;
  put "\n";

  y.a := true;
  put "printing a partially initialised record...\n";
  put y;
  put "\n";

  y.b := true;
  put "printing a fully initialised record...\n";
  put y;
  put "\n";

  put "printing an uninitialised array...\n";
  put z;
  put "\n";

  for i: -4 .. 1 do z[i] := true; end;
  put "printing a partially initialised array\n";
  put z;
  put "\n";

  for i: 2 .. 7 do z[i] := false; end;
  put "printing a fully initialised array\n";
  put z;
  put "\n";

end;

rule begin
  x := !x;
end;
