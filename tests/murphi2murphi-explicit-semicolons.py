#!/usr/bin/env python3

import os
import pathlib
import re
import subprocess

# pick an arbitrary test model that omits semi-colons
model = pathlib.Path(__file__).parent / 'assume-statement.m'
assert model.exists()

# use the explicit-semicolons pass to add semi-colons
print(f'+ murphi2murphi --explicit-semicolons {model}')
transformed = subprocess.check_output(['murphi2murphi',
  '--explicit-semicolons', model])
decoded = transformed.decode('utf-8', 'replace')

print(f'transformed model:\n{decoded}')

# we should now find the variable definition and both rules in this model are
# semi-colon terminated
assert re.search(r'\bx: 0 \.\. 2;$', decoded, re.MULTILINE)
assert len(re.findall(r'^end;$', decoded, re.MULTILINE)) == 2

# the generated model also should be valid syntax for Rumur
print('+ rumur --output /dev/null <(transformed model)')
subprocess.run(['rumur', '--output', os.devnull], check=True, input=transformed)
