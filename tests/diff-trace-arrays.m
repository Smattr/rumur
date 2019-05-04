-- rumur_flags: ['--deadlock-detection', 'off', '--threads', '1']
-- checker_exit_code: 1
-- checker_output: None if xml else re.compile(r'(?!(.|\n)*\b(s\[D\]:\s*3)\b(.|\n)*\b\2\b)')

/* This model tests for regression of a bug first observed on commit
 * 7e5654ddb9ad4f3342d3c1b1abef9a3402df5dee. Certain models using enums to index
 * arrays which produced a failing counter-example, would result in a diff trace
 * that repeatedly listed an unchanging value for some members of the array. If
 * the bug has been re-introduced, this model will print the assignment 'S[D]:3'
 * multiple times in the trace.
 */

type
  t: enum { A, B, C, D, E };
  d: 0 .. 4;

var
  s: array[t] of d;

startstate begin
end;

ruleset i: t; v: d do
  rule isundefined(s[i]) & forall j: t do j = i | isundefined(s[j]) | s[j] < v end ==> begin
    s[i] := v;
  end;
end;

invariant exists i: t do isundefined(s[i]) end
