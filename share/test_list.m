dnl basic tester for list.m
dnl
dnl This test model aims to cover the state space for a typically usage of the
dnl generic list. Any mistakes in implementation should trigger either an
dnl assertion failure, a read/write out of bounds, or a read of an undefined
dnl value.
const
  LEN: 5;

type
  int: 0..1;

dnl a list of integers, replicating typical usage
define(`name', `ints')
define(`index_t', `0..LEN - 1')
define(`elem_t', `int')
include(`list.m')
undefine(`elem_t')
undefine(`index_t')
undefine(`name')

var
  xs: ints_t;
  -- bookkeep the occupancy of xs for checking
  shadow_length: 0..LEN;
  -- mirror the contents of xs for checking
  shadow: array[0..LEN - 1] of int;

-- copy the contents of an instantiated list into a raw list, accounting for
-- possibly undefined elements
procedure copy(var dst: array[0..LEN - 1] of int; src: ints_t);
begin
  undefine dst;
  for i: 0..LEN - 1 do
    if !isundefined(src[i].is_populated_) then
      dst[i] := src[i].value_;
    end;
  end;
end;

-- compare the contents of an instantiated list to a raw list, accounting for
-- possibly undefined elements
function is_equal(as: array[0..LEN - 1] of int; bs: ints_t): boolean;
begin
  for i: 0..LEN - 1 do
    if isundefined(as[i]) then
      if !isundefined(bs[i].is_populated_) then
        return false;
      end;
    else
      if isundefined(bs[i].is_populated_) then
        return false;
      end;
      if as[i] != bs[i].value_ then
        return false;
      end;
    end;
  end;
  return true;
end;

startstate
begin
  shadow_length := 0;
end;

ruleset x: int do
  rule "push_back" !ints_is_full(xs) ==>
  var
    before: array[0..LEN - 1] of int;
  begin
    assert is_equal(shadow, xs);
    copy(before, xs);

    ints_push_back(xs, x);
    copy(shadow, xs);

    -- check the push had the effect we expect
    for i: 0..LEN - 1 do
      if i < shadow_length then
        assert before[i] = shadow[i];
      elsif i = shadow_length then
        assert isundefined(before[i]);
        assert shadow[i] = x;
      else
        assert isundefined(before[i]);
        assert isundefined(shadow[i]);
      end;
    end;

    shadow_length := shadow_length + 1;
  end;

  rule "try_push_back"
  var
    before: array[0..LEN - 1] of int;
  begin
    assert is_equal(shadow, xs);
    copy(before, xs);

    if ints_try_push_back(xs, x) then
      copy(shadow, xs);

      -- check the push had the effect we expect
      for i: 0..LEN - 1 do
        if i < shadow_length then
          assert before[i] = shadow[i];
        elsif i = shadow_length then
          assert isundefined(before[i]);
          assert shadow[i] = x;
        else
          assert isundefined(before[i]);
          assert isundefined(shadow[i]);
        end;
      end;

      shadow_length := shadow_length + 1;
    end;
  end;
end;

rule "pop_back" !ints_is_empty(xs) ==>
var
  before: array[0..LEN - 1] of int;
begin
  assert is_equal(shadow, xs);
  copy(before, xs);

  ints_pop_back(xs);
  copy(shadow, xs);

  -- check the pop had the effect we expect
  for i: 0..LEN - 1 do
    if i < shadow_length - 1 then
      assert before[i] = shadow[i];
    elsif i = shadow_length - 1 then
      assert !isundefined(before[i]);
      assert isundefined(shadow[i]);
    else
      assert isundefined(before[i]);
      assert isundefined(shadow[i]);
    end;
  end;

  shadow_length := shadow_length - 1;
end;

rule "pop_front" !ints_is_empty(xs) ==>
var
  before: array[0..LEN - 1] of int;
begin
  assert is_equal(shadow, xs);
  copy(before, xs);

  ints_pop_front(xs);
  copy(shadow, xs);

  -- check the pop had the effect we expect
  for i: 0..LEN - 1 do
    if i < shadow_length - 1 then
      assert before[i + 1] = shadow[i];
    elsif i = shadow_length - 1 then
      assert !isundefined(before[i]);
      assert isundefined(shadow[i]);
    else
      assert isundefined(before[i]);
      assert isundefined(shadow[i]);
    end;
  end;

  shadow_length := shadow_length - 1;
end;
