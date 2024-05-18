#!/usr/bin/env python3

import os
import pathlib
import re
import subprocess

# model from the test directory involving a record comparison
model = pathlib.Path(__file__).parent / "compare-record.m"
assert model.exists()

# use the complex comparison decomposition to explode the record comparison in
# this model
print("+ murphi2murphi --decompose-complex-comparisons {}".format(model))
transformed = subprocess.check_output(["murphi2murphi",
  "--decompose-complex-comparisons", str(model)])
decoded = transformed.decode("utf-8", "replace")

print("transformed model:\n{}".format(decoded))

# the comparisons should have been decomposed into member-wise comparison
assert re.search(r"\bx\.x = y\.x\b", decoded)
assert re.search(r"\bx\.x != y\.x\b", decoded)

# the generated model also should be valid syntax for Rumur
print("+ rumur --output /dev/null <(transformed model)")
p = subprocess.Popen(["rumur", "--output", os.devnull], stdin=subprocess.PIPE)
p.communicate(transformed)
assert p.returncode == 0
