#!/usr/bin/env python3

import os
import pathlib
import re
import subprocess

# model from the test directory involving some switch examples
model = pathlib.Path(__file__).parent / 'switch-stmt2.m'
assert model.exists()

# use the switch-to-if pass to remove the switch statements
print(f'+ murphi2murphi --switch-to-if {model}')
transformed = subprocess.check_output(['murphi2murphi',
  '--switch-to-if', model])
decoded = transformed.decode('utf-8', 'replace')

print(f'transformed model:\n{decoded}')

# we should now be able to find some if statements in the model
assert re.search(r'\bif x = y then\b', decoded)
assert re.search(r'\belsif x = 5 then\b', decoded)

# the generated model also should be valid syntax for Rumur
print('+ rumur --output /dev/null <(transformed model)')
subprocess.run(['rumur', '--output', os.devnull], check=True, input=transformed)
