-- checker_exit_code: 1
-- checker_output: re.compile(r'^Rule Rule 0, y:\s*(true|false) fired.$', re.MULTILINE)

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
