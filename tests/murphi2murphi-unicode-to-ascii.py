#!/usr/bin/env python3

import os
import pathlib
import re
import subprocess

# model from the test directory involving a record comparison
model = pathlib.Path(__file__).parent / 'unicode-assignment.m'
assert model.exists()

# use the ASCII transformation to remove ≔
print(f'+ murphi2murphi --to-ascii {model}')
transformed = subprocess.check_output(['murphi2murphi',
  '--to-ascii', model])
decoded = transformed.decode('utf-8', 'replace')

print(f'transformed model:\n{decoded}')

# the ≔ operator should have become :=
assert re.search(r'\bx := true\b', decoded)
assert re.search(r'\bx := !x\b', decoded)

# the generated model also should be valid syntax for Rumur
print('+ rumur --output /dev/null <(transformed model)')
subprocess.run(['rumur', '--output', os.devnull], check=True, input=transformed)
