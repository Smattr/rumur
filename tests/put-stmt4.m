-- Test put statements get parsed correctly.

var
  x: boolean;
  y: record
    a: boolean;
    b: boolean;
  end;

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

end;

rule begin
  x := !x;
end;
