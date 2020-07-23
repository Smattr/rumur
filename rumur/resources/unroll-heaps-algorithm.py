#!/usr/bin/env python3

'''
generator for unrolled versions of Heap's Algorithm

Tracking successive scalarset permutations during verification is done using the
functions permutation_to_index() and index_to_permutation() in header.c, that
derive a two way mapping between numbers and permutations. The implementation of
these two functions follows Heap's Algorithm for iterating over permutations.
However, this was found to be quite expensive when initially implemented (~18%
performance overhead during verification).

Taking advantage of the fact that scalarsets of size >5 are rarely used in real
world models, we can specialise the mapping functions by statically unrolling
Heap's Algorithm to simply linearly iterate over known permutations. The
following script can be used to generate C versions of such specialisations.

This is not called during compilation of Rumur, but was rather run once, and its
output copied into header.c.
'''

import argparse
import functools
import math
import sys
from typing import Any, Dict, Generator, Tuple

def w(s: str) -> None:
  '''a shortcut for printing'''
  sys.stdout.write(s)

def heap(count: int) -> Generator[Tuple[int, Tuple[int, ...]], None, None]:
  '''
  Heap's Algorithm. This is intended to exactly mimic the implementation logic
  in header.c.
  '''

  index = 0
  stack = [0] * count
  permutation = list(range(count))

  yield index, tuple(permutation)

  i = 0
  while i < count:
    if stack[i] < i:
      index += 1
      if i % 2 == 0:
        permutation[0], permutation[i] = permutation[i], permutation[0]
      else:
        permutation[stack[i]], permutation[i] \
          = permutation[i], permutation[stack[i]]
      yield index, tuple(permutation)
      stack[i] += 1
      i = 0
    else:
      stack[i] = 0
      i += 1

def make_dict(count: int) -> Dict[int, Any]:
  '''
  construct a hierarchical dictionary representation of all permutations of the
  given count
  '''

  permutations: Dict[int, Any] = {}

  for i, perm in heap(count):
    d = permutations
    for j in perm[:-1]:
      if j not in d:
        d[j] = {}
      d = d[j]
    d[perm[-1]] = i

  return permutations

def w_dict(permutations: Dict[int, Any], count: int, indentation: int) -> None:
  '''
  print the contents of a permutation dictionary (see above)
  '''

  # if this is the last level of the dictionary, we can print (the sole) element
  if all(isinstance(x, int) for x in permutations.values()):
    assert len(permutations) == 1
    w(f'{list(permutations.values())[0]}')
    return

  # otherwise, another level of indentation
  w(f'{{\n')
  for k in sorted(permutations.keys()):
    w(f'{(indentation + 1) * 2 * " "}[{k}] = ')
    w_dict(permutations[k], count, indentation + 1)
    w(',\n')
  w(f'{indentation * 2 * " "}}}')

def main(args: [str]) -> int:

  # parse command line arguments
  parser = argparse.ArgumentParser(
    description='generator for an unrolled Heap\'s Algorithm')
  parser.add_argument('limit', type=int, help='maximum size of array to handle')
  options = parser.parse_args(args[1:])

  # generate permutation -> index function
  w('/* unrolled version of permutation_to_index() generated by\n'
    ' * unroll-heaps-algorithm.py\n'
    ' */\n'
    'static size_t permutation_to_index_unrolled(const size_t *NONNULL '
      'permutation,\n'
    '    size_t count) {\n'
    '\n'
   f'  ASSERT(count <= {options.limit});\n'
    '\n'
    '  switch (count) {\n'
    '\n')
  for limit in range(2, options.limit + 1):
    w(f'    case {limit}: {{\n')

    # construct a lookup table, mapping permutation to index
    w(f'      static const size_t index{f"[{limit}]" * (limit - 1)} = ')
    perms = make_dict(limit)
    w_dict(perms, limit, 3)
    w(';\n')

    # emit the lookup itself
    w('      return index'
     f'{"".join(f"[permutation[{i}]]" for i in range(limit - 1))};\n'
      '    }\n'
      '\n')
  w('  }\n'
    '\n'
    '  ASSERT(!"invalid call to permutation_to_index_unrolled()");\n'
    '}\n'
    '\n')

  # generate index -> permutation function
  w('/* unrolled version of index_to_permutation() generated by\n'
    ' * unroll-heaps-algorithm.py\n'
    ' */\n'
    'static const size_t *index_to_permutation_unrolled(size_t index,\n'
    '    size_t count) {\n'
    '\n'
   f'  ASSERT(count <= {options.limit});\n'
    '\n'
    '  switch (count) {\n'
    '\n')
  for limit in range(2, options.limit + 1):
    w(f'    case {limit}: {{\n')

    # construct a lookup table, mapping index to permutation
    w('      static const size_t permutation'
     f'[{math.factorial(limit)}][{limit}] = {{\n')
    for _, permutation in heap(limit):
      w(f'        {{ {", ".join(str(p) for p in permutation)} }},\n')
    w('      };\n')

    # emit the lookup itself
    w('      return permutation[index];\n'
      '    }\n'
      '\n')

  w('  }\n'
    '\n'
    '  ASSERT(!"invalid call to index_to_permutation_unrolled()");\n'
    '}\n'
    '\n')

  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv))
