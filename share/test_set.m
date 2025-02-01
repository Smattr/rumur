dnl basic tester for ./_set
dnl
dnl This test model aims to cover the state space for a typically usage of the
dnl generic set. Any mistakes in implementation should trigger either an
dnl assertion failure, a read/write out of bounds, or a read of an undefined
dnl value.
include(`_set')dnl

-------------------------------------------------------------------------------
-- a set of integers, replicating typical usage
-------------------------------------------------------------------------------

type
  int: 0..5;

_set(`ints', `int')dnl

var
  xs: ints_t;

startstate
begin
  assert ints_is_empty(xs);
  assert !ints_is_full(xs);
  assert forall x: int do !ints_contains(xs, x) end;
end;

ruleset i: int do
  rule "add"
  var
    noop: boolean;
    rc: boolean;
  begin
    noop := ints_contains(xs, i);
    rc := ints_add(xs, i);
    assert noop = rc;
    assert !ints_is_empty(xs);
  end;

  rule "remove"
  var
    noop: boolean;
    rc: boolean;
  begin
    noop := !ints_contains(xs, i);
    rc := ints_remove(xs, i);
    assert !noop = rc;
    assert !ints_is_full(xs);
  end;
end;

invariant exists x: int do ints_contains(xs, x) end -> !ints_is_empty(xs);
invariant forall x: int do !ints_contains(xs, x) end -> ints_is_empty(xs);
invariant exists x: int do !ints_contains(xs, x) end -> !ints_is_full(xs);
invariant forall x: int do ints_contains(xs, x) end -> ints_is_full(xs);
