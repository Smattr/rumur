-- checker_exit_code: 1
-- checker_output: None if xml else re.compile(r'^Rule\s+1, y:\s*(true|false) fired.$', re.MULTILINE)

-- Similar to cex-enum.m, but with the built-in enum 'boolean'.

var
  x: boolean;

startstate begin
  x := true;
end;

ruleset y: boolean do
  rule begin
    x := !x;
  end;
end;

invariant x;
