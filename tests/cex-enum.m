-- checker_exit_code: 1
-- checker_output: None if xml else re.compile(r'^Rule\s+1, y:\s*[AB] fired.$', re.MULTILINE)

/* An example that causes a counter-example trace involving an enum. The
 * motivation for this is that rulesets quantifying over enums led to enums
 * being printed as numeric values instead of the enum member (observed on
 * commit 8bf648a180db4070cc5777dd5c73f1a4635c0ab1).
 */

type
  t: enum { A, B};

var
  x: boolean;

startstate begin
  x := true;
end;

ruleset y: t do
  rule begin
    x := !x;
  end;
end;

invariant x;
