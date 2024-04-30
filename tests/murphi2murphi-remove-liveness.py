#!/usr/bin/env python3

import os
import pathlib
import subprocess

# an arbitrary test model that has a liveness property
model = pathlib.Path(__file__).parent / "liveness-miss1.m"
assert model.exists()

# confirm it contains the liveness property we expect
with open(str(model), "rt", encoding="utf-8") as f:
  assert 'liveness "x is 10" x = 10' in f.read()

# use the remove-liveness pass to remove the property
print("+ murphi2murphi --remove-liveness {}".format(model))
transformed = subprocess.check_output(["murphi2murphi",
  "--remove-liveness", str(model)])
decoded = transformed.decode("utf-8", "replace")

print("transformed model:\n{}".format(decoded))

# now confirm the property is no longer present
assert 'liveness "x is 10" x = 10' not in decoded

# the generated model also should be valid syntax for Rumur
print("+ rumur --output /dev/null <(transformed model)")
p = subprocess.Popen(["rumur", "--output", os.devnull], stdin=subprocess.PIPE)
p.communicate(transformed)
assert p.returncode == 0
