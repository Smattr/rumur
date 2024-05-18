#!/usr/bin/env python3

import os
import pathlib
import re
import subprocess

# model from the test directory involving some switch examples
model = pathlib.Path(__file__).parent / "switch-stmt1.m"
assert model.exists()

# use the switch-to-if pass to remove the switch statements
print("+ murphi2murphi --switch-to-if {}".format(model))
transformed = subprocess.check_output(["murphi2murphi",
  "--switch-to-if", str(model)])
decoded = transformed.decode("utf-8", "replace")

print("transformed model:\n{}".format(decoded))

# we should now be able to find some if statements in the model
assert re.search(r"\bif x = 1 then\b", decoded)
assert re.search(r"\belsif \(?x = 3\)? | \(?x = 4\)? then\b", decoded)

# the generated model also should be valid syntax for Rumur
print("+ rumur --output /dev/null <(transformed model)")
p = subprocess.Popen(["rumur", "--output", os.devnull], stdin=subprocess.PIPE)
p.communicate(transformed)
assert p.returncode == 0
