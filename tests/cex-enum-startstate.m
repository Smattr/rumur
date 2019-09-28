-- checker_exit_code: 1
-- checker_output: None if xml else re.compile(r'^Startstate\s+1, y:\s*[AB] fired.$', re.MULTILINE)

-- Similar to cex-enum.m, but with the enum parameter in a startstate.

type
  t: enum { A, B};

var
  x: boolean;

ruleset y: t do
  startstate begin
    x := true;
  end;
end;

rule begin
  x := !x;
end;

invariant x;
