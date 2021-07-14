#!/usr/bin/env python3

import os
import pathlib
import re
import subprocess

# an arbitrary test model that has a liveness property
model = pathlib.Path(__file__).parent / 'liveness-miss1.m'
assert model.exists()

# confirm it contains the liveness property we expect
with open(model, 'rt', encoding='utf-8') as f:
  assert re.search(r'liveness "x is 10" x = 10', f.read())

# use the remove-liveness pass to remove the property
print('+ murphi2murphi --remove-liveness {}'.format(model))
transformed = subprocess.check_output(['murphi2murphi',
  '--remove-liveness', model])
decoded = transformed.decode('utf-8', 'replace')

print('transformed model:\n{}'.format(decoded))

# now confirm the property is no longer present
assert re.search(r'liveness "x is 10" x = 10', decoded) is None

# the generated model also should be valid syntax for Rumur
print('+ rumur --output /dev/null <(transformed model)')
p = subprocess.Popen(['rumur', '--output', os.devnull], stdin=subprocess.PIPE)
p.communicate(transformed)
assert p.returncode == 0
