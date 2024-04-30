#!/usr/bin/env python3

import os
import pathlib
import re
import subprocess

# model from the test directory involving an array comparison
model = pathlib.Path(__file__).parent / "compare-array.m"
assert model.exists()

# use the complex comparison decomposition to explode the array comparison in
# this model
print("+ murphi2murphi --decompose-complex-comparisons {}".format(model))
transformed = subprocess.check_output(["murphi2murphi",
  "--decompose-complex-comparisons", str(model)])
decoded = transformed.decode("utf-8", "replace")

print("transformed model:\n{}".format(decoded))

# the comparisons should have been decomposed into element-wise comparison
assert re.search(r"\bx\[i0\] = y\[i0\]", decoded)
assert re.search(r"\bx\[i0\] != y\[i0\]", decoded)

# the generated model also should be valid syntax for Rumur
print("+ rumur --output /dev/null <(transformed model)")
p = subprocess.Popen(["rumur", "--output", os.devnull], stdin=subprocess.PIPE)
p.communicate(transformed)
assert p.returncode == 0
