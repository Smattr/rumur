#!/usr/bin/env python3

"""
`murphi2uclid` previously had a bug wherein the `--numeric-type` command-line
option was ignored. The following tests whether this bug has been reintroduced.
"""

import re
import subprocess
import sys

MODEL = """
const N: 2;
var x: 0 .. 1;
startstate begin
  x := 0;
end;
rule begin
  x := 1 - x;
end;
"""

# translate a model to Uclid5 requesting a non-default bit-vector type
uclid = subprocess.check_output(
    ["murphi2uclid", "--numeric-type=bv64"], input=MODEL, universal_newlines=True
)

# this should have been used for (at least) the constant
if re.search(r"\bconst\s+N\s*:\s*bv64\b", uclid) is None:
    sys.stderr.write("Numeric type 'bv64' not used in output:\n{}".format(uclid))
    sys.exit(-1)

sys.exit(0)
