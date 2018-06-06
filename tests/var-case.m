-- This test is designed to provoke any issues with handling the
-- case-sensitivity of variables.

var
  state: boolean;
  State: boolean;
  sTate: boolean;
  stAte: boolean;
  staTe: boolean;
  statE: boolean;
  STATE: boolean;

startstate begin
  state := true;
  State := false;
end;

rule begin
  state := !state;
  State := !State;
end;

invariant
  state != State;
