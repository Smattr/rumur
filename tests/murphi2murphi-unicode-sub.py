#!/usr/bin/env python3

import os
import pathlib
import re
import subprocess

# model from the test directory involving a unicode subtraction
model = pathlib.Path(__file__).parent / "unicode-sub.m"
assert model.exists()

# use the ASCII transformation to remove −
print("+ murphi2murphi --to-ascii {}".format(model))
transformed = subprocess.check_output(["murphi2murphi",
  "--to-ascii", str(model)])
decoded = transformed.decode("utf-8", "replace")

print("transformed model:\n{}".format(decoded))

# the − operator should have become -
assert re.search(r"\bx := 1 - x\b", decoded)
assert "−" not in decoded

# the generated model also should be valid syntax for Rumur
print("+ rumur --output /dev/null <(transformed model)")
p = subprocess.Popen(["rumur", "--output", os.devnull], stdin=subprocess.PIPE)
p.communicate(transformed)
assert p.returncode == 0
