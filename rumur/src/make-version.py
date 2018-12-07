#!/usr/bin/env python

'''
Generate contents of a version.cc.
'''

import os, re, subprocess, sys

def main(args):

  if len(args) != 2 or args[1] == '--help':
    sys.stderr.write(
      'usage: {} file\n'
      ' write version information as a C++ source file\n'.format(args[0]))
    return -1

  old = None

  if os.path.exists(args[1]):
    with open(args[1], 'rt') as f:
      old = f.read()

  # First, look for a version tag on the current commit.
  try:
    with open(os.devnull, 'wt') as f:
      tag = subprocess.check_output(['git', 'describe', '--tags'], stderr=f)
  except subprocess.CalledProcessError:
    tag = None
  if tag is not None:
    tag = tag.decode('utf-8', 'replace').strip()
    if re.match(r'v[\d\.]+$', tag) is None:
      # Not a version tag.
      tag = None

  if tag is None:
    # We didn't find a version tag. Use the commit hash as the version.
    rev = subprocess.check_output(['git', 'rev-parse', '--verify', 'HEAD'])
    rev = rev.decode('utf-8', 'replace').strip()

    version = 'Git commit {}'.format(rev)

  else:
    version = tag

  dirty = False

  with open(os.devnull, 'wt') as f:

    p = subprocess.Popen(['git', 'diff', '--exit-code'], stdout=f, stderr=f)
    p.communicate()
    dirty |= p.returncode != 0

    p = subprocess.Popen(['git', 'diff', '--cached', '--exit-code'], stdout=f,
      stderr=f)
    p.communicate()
    dirty |= p.returncode != 0

  new = 'const char *VERSION = "{}{}";\n'.format(version,
    ' (dirty)' if dirty else '')

  # If the version has changed, update the output. Otherwise we leave the old
  # contents -- and more importantly, the timestamp -- intact.
  if old != new:
    with open(args[1], 'wt') as f:
      f.write(new)

  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv))
