#!/usr/bin/env python

'''
Generate contents of a version.cc.
'''

import os, re, subprocess, sys

def get_tag():
  '''
  Find the version tag of the current Git commit, e.g. v2020.05.03, if it
  exists.
  '''
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

  return tag

def get_sha():
  '''
  Find the hash of the current Git commit.
  '''
  rev = subprocess.check_output(['git', 'rev-parse', '--verify', 'HEAD'])
  rev = rev.decode('utf-8', 'replace').strip()

  return rev

def is_dirty():
  '''
  Determine whether the current working directory has uncommitted changes.
  '''
  dirty = False

  with open(os.devnull, 'wt') as f:

    p = subprocess.Popen(['git', 'diff', '--exit-code'], stdout=f, stderr=f)
    p.communicate()
    dirty |= p.returncode != 0

    p = subprocess.Popen(['git', 'diff', '--cached', '--exit-code'], stdout=f,
      stderr=f)
    p.communicate()
    dirty |= p.returncode != 0

  return dirty

def main(args):

  if len(args) != 2 or args[1] == '--help':
    sys.stderr.write(
      'usage: {} file\n'
      ' write version information as a C++ source file\n'.format(args[0]))
    return -1

  # Get the contents of the old version file if it exists.
  old = None
  if os.path.exists(args[1]):
    with open(args[1], 'rt') as f:
      old = f.read()

  version = None

  # First, look for a version tag on the current commit.
  if version is None:
    tag = get_tag()
    if tag is not None:
      version = '{}{}'.format(tag, ' (dirty)' if is_dirty() else '')

  # If we didn't find a version tag. Use the commit hash as the version.
  if version is None:
    rev = get_sha()
    assert rev is not None
    version = 'Git commit {}{}'.format(rev, ' (dirty)' if is_dirty() else '')

  new = 'const char *VERSION = "{}";\n'.format(version)

  # If the version has changed, update the output. Otherwise we leave the old
  # contents -- and more importantly, the timestamp -- intact.
  if old != new:
    with open(args[1], 'wt') as f:
      f.write(new)

  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv))
