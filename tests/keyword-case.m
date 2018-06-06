-- This test is designed to provoke any issues with handling the
-- case-insensitivity of keywords.

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin x := !x; end;
Rule begin x := !x; end;
rUle begin x := !x; end;
ruLe begin x := !x; end;
rulE begin x := !x; end;
RULE begin x := !x; end;
