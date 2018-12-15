/* The following model contains a simple rule with no 'begin' token. According
 * to the Murphi grammar [0] this is valid, but neither CMurphi nor Rumur at
 * time of writing (commit 5222f6ddce51ea66ceda6ecb0e016a94308e835b) are able to
 * parse this model. The root cause is that this part of the grammar is
 * ambiguous to an LALR(1) parser, which is what both CMurphi and Rumur are
 * using. It would be nice to address this weakness in future, but to do so I
 * think we will need to move to a GLR parser.
 *
 * Github: see also #77 "begin on rules is not optional"
 *
 *   [0]: https://www.cs.ubc.ca/~ajh/courses/cpsc513/assign-token/User.Manual
 */

var
  x: boolean;

startstate
  x := true;
end;

rule
  x := !x;
end;
