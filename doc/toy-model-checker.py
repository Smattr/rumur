#!/usr/bin/env python3

"""
The following is a simplified model checker for a specific problem. The aim is
to help readers understand how a model checker like Rumur works. It is written
in Python for readability, however the Rumur implementation is generated C code
optimised for both memory usage and execution speed. If you have read and
understood the following file, you will be in a position to start reading and
understanding ../rumur/resources/header.c.
"""

import sys
from typing import Optional, Set

class State(object):

  def __init__(self, value: int = 0, previous: Optional["State"] = None):

    # the state of our model; a single integer
    self.value = value

    # a pointer to the previous state, for the purpose of generating counter
    # example traces
    self.previous = previous

  def duplicate(self) -> "State":
    """create a new State that will be a successor of this one"""
    return State(self.value, self)

  # make objects of this type storable in a set
  def __hash__(self) -> int:
    return self.value


# an invariant we will claim, that the state never exceeds 10
def invariant(s: State) -> bool:
  return s.value <= 10


# two rules and their accompanying guards
def inc2_guard(s: State) -> bool:
  return s.value < 20
def inc2_rule(s: State) -> None:
  s.value += 2
def dec1_guard(s: State) -> bool:
  return s.value > 0
def dec1_rule(s: State) -> None:
  s.value -= 1


# a start state that initially sets our value to 0
def start(s: State) -> None:
  s.value = 0


def print_cex(s: State) -> None:
  """print a counter example trace ending at the given state"""

  if s.previous is not None:
    print_cex(s.previous)

  # Print the value of this state. In a real world model checker like Rumur, you
  # would typically also print the transition rule that connected this state to
  # the previous to aid debugging.
  print(f" value == {s.value}")


def main() -> int:

  # A queue of states to be expanded and explored. Checking is done when this
  # queue is exhausted.
  pending: [State] = []

  # A set of states we have already encountered. We use this to deduplicate
  # paths during exploration and avoid repeatedly checking the same states.
  seen: Set[State] = set()

  # we start with just our starting state in the queue
  s = State()
  start(s)
  pending.append(s)

  while len(pending) > 0:

    # take the next state off the queue to be expanded
    n = pending.pop(0)

    # try each rule on it to generate new states
    for guard, rule in ((inc2_guard, inc2_rule), (dec1_guard, dec1_rule)):

      if guard(n):
        e = n.duplicate()
        rule(e)

        # Here is where symmetry reduction steps would usually take place in a
        # real model checker. These are used to further deduplicate states and
        # reduce the state space. For the purposes of this example, we omit such
        # optimisations.

        # Have we seen this new state before? If so, we can discard it as a
        # duplicate.
        if e in seen:
          continue

        # Does the state violate our invariant?
        if not invariant(e):
          print("counter example trace:")
          print_cex(e)
          return -1

        # note that we have seen this state for future checks
        seen.add(e)

        # add it to our queue and proceed
        pending.append(e)


  print("checking complete")

  return 0

if __name__ == "__main__":
  sys.exit(main())
