#!/usr/bin/env python3

import os
import pathlib
import re
import subprocess

# model from the test directory involving a record comparison
model = pathlib.Path(__file__).parent / 'compare-record.m'
assert model.exists()

# use the complex comparison decomposition to explode the record comparison in
# this model
print(f'+ murphi2murphi --decompose-complex-comparisons {model}')
transformed = subprocess.check_output(['murphi2murphi',
  '--decompose-complex-comparisons', model])
decoded = transformed.decode('utf-8', 'replace')

print(f'transformed model:\n{decoded}')

# the comparisons should have been decomposed into member-wise comparison
assert re.search(r'\bx\.x = y\.x\b', decoded)
assert re.search(r'\bx\.x != y\.x\b', decoded)

# the generated model also should be valid syntax for Rumur
print('+ rumur --output /dev/null <(transformed model)')
subprocess.run(['rumur', '--output', os.devnull], check=True, input=transformed)
