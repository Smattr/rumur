-- checker_exit_code: 1
-- checker_output: re.compile(r'^Startstate Startstate 0, y:\s*(true|false) fired.$', re.MULTILINE)

-- Similar to cex-boolean.m, but using a ruleset startstate.

var
  x: boolean;

ruleset y: boolean do
  startstate begin
    x := true;
  end;
end;

rule begin
  x := !x;
end;

invariant x;
