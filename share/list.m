dnl template for a statically sized list/queue/stack
dnl
dnl This file defines, as an M4¹ template, a type-generic Murphi list data
dnl structure. To use this, define the macros "name", "index_t", and "elem_t":
dnl   • "name" – a prefix for the generated list data structure and its
dnl     supporting functions
dnl   • "index_t" – a Murphi type for indexing the list
dnl   • "elem_t" – a Murphi type for the elements of the list
dnl and then include this file, e.g.:
dnl
dnl   type
dnl     int: 0..32;
dnl
dnl   define(`name', `ints')dnl
dnl   define(`index_t', `0..10')dnl
dnl   define(`elem_t', `int')dnl
dnl   include(`list.m')dnl
dnl   undefine(`elem_t')dnl
dnl   undefine(`index_t')dnl
dnl   undefine(`name')dnl
dnl
dnl Pre-process your model with M4:
dnl
dnl   $ m4 --include=/path/to/rumur/install/share/rumur/lib model.m >out.m
dnl
dnl This generates a type and some supporting functions:
dnl
dnl   type
dnl     ints_t: …
dnl
dnl   function ints_is_empty(list_: ints_t): boolean;
dnl   function ints_is_full(list_: ints_t): boolean;
dnl   procedure ints_push_back(var list_: ints_t; elem_: int);
dnl   function ints_try_push_back(var list_: ints_t; elem_: int): boolean;
dnl   function ints_pop_front(var list_: ints_t): int;
dnl   function ints_pop_back(var list_: ints_t): int;
dnl
dnl You can then go on to use the "ints_t" type in your handwritten model code.
dnl
dnl "index_t" can be an arbitrary type; the only requirement is that it is
dnl iterable. It can be a previously defined type or a type literal. The
dnl underlying type could be boolean, a range, a scalarset, or an enum. Though
dnl it is unlikely anything except a range makes sense.
dnl
dnl "elem_t" can be an arbitrary type. It can be a previously defined type or a
dnl type literal, though you probably want to name it in order to more easily
dnl use the push and pop functions.
dnl
dnl This template is in the public domain. You may use it for any purpose and
dnl its inclusion in a model does not affect the legal status of that model.
dnl
dnl ¹ https://en.wikipedia.org/wiki/M4_(computer_language)
dnl
ifdef(`name', `', `errprint(the macro "name" must be defined before including list.m
)m4exit(1)')dnl
ifdef(`index_t', `', `errprint(the macro "index_t" must be defined before including list.m
)m4exit(1)')dnl
ifdef(`elem_t', `', `errprint(the macro "elem_t" must be defined before including list.m
)m4exit(1)')dnl
-------------------------------------------------------------------------------
-- interface for name`'_t, a list of elem_t values indexed by index_t
-------------------------------------------------------------------------------

type
  name`'_t: array[index_t] of record
    -- Does this slot hold an item? The type 1..1 is used to model a boolean
    -- without incurring an extra bit for the undefined value. I.e.
    --   false = isundefined(is_populated_)
    --   true = !isundefined(is_populated_)
    is_populated_: 1..1;
    -- the contents of the slot, if !isundefined(is_populated_)
    value_: elem_t;
  end;

function name`'_is_empty(list_: name`'_t): boolean;
begin
  -- use a single-iteration loop to check the first element to avoid assuming
  -- the lower bound of type index_t
  for i_: index_t do
    if !isundefined(list_[i_].is_populated_) then
      return false;
    else
      return true;
    end;
  end;
  -- in the edge case where index_t is an empty type, consider the list always
  -- empty
  return true;
end;

function name`'_is_full(list_: name`'_t): boolean;
begin
  -- use a loop to check the last element to avoid assuming the upper bound of
  -- type index_t
  for i_: index_t do
    if isundefined(list_[i_].is_populated_) then
      return false;
    end;
  end;
  return true;
end;

-- Treating a name`'_t as a first-in-first-out queue, enqueue an item. Or
-- equivalently, treating a name`'_t as a last-in-first-out stack, stack an
-- item.
procedure name`'_push_back(var list_: name`'_t; elem_: elem_t);
begin
  assert "attempting to push into a full name`'_t" !name`'_is_full(list_);
  -- use a loop to find the first empty slot to avoid assuming the upper bound
  -- of type index_t
  for i_: index_t do
    if isundefined(list_[i_].is_populated_) then
      list_[i_].value_ := elem_;
      list_[i_].is_populated_ := 1;
      return;
    end;
  end;
  assert "unreachable code executed" false;
end;

-- Treating a name`'_t as a first-in-first-out queue, enqueue an item and return
-- true if possible. Or equivalently, treating a name`'_t as a last-in-first-out
-- stack, stack an item and return true if possible. If it is not possible to
-- insert the new item, do nothing and return false.
function name`'_try_push_back(var list_: name`'_t; elem_: elem_t): boolean;
begin
  if name`'_is_full(list_) then
    return false;
  end;
  name`'_push_back(list_, elem_);
  return true;
end;

-- treating a name`'_t as a first-in-first-out queue, dequeue an item
function name`'_pop_front(var list_: name`'_t): elem_t;
var
  first_: elem_t;
  successor_: boolean;
begin
  assert "attempting to pop from an empty name`'_t" !name`'_is_empty(list_);
  for i_: index_t do

    -- extract the first element
    assert "corrupted list" !isundefined(list_[i_].is_populated_);
    first_ := list_[i_].value_;

    -- Shuffle the remaining elements forwards. We do this with an unorthodox
    -- double loop to avoid assuming anything about the bounds of index_t or
    -- even whether its members are orderable.
    for j_: index_t do
      undefine list_[j_];
      successor_ := false;
      for k_: index_t do
        if j_ = k_ then
          assert "incorrect shuffle logic" !successor_;
          successor_ := true;
        elsif successor_ then
          list_[j_] := list_[k_];
          successor_ := false;
        end;
      end;
    end;

    return first_;
  end;
end;

-- treating a name`'_t as a last-in-first-out stack, unstack an item
function name`'_pop_back(var list_: name`'_t): elem_t;
var
  last_: elem_t;
  has_successor_: boolean;
  successor_: boolean;
begin
  assert "attempting to pop from an empty name`'_t" !name`'_is_empty(list_);
  for i_: index_t do

    -- Is this element the last? We do this with an unorthodox loop to avoid
    -- assuming anything about the bounds of index_t or even whether its members
    -- are orderable.
    has_successor_ := false;
    for j_: index_t do
      if i_ = j_ then
        successor_ := true;
      elsif successor_ then
        has_successor_ := !isundefined(list_[j_].is_populated_);
        successor_ := false;
      end;
    end;

    -- if this is the last element, extract and return it
    if !has_successor_ then
      last_ := list_[i_].value_;
      undefine list_[i_];
      return last_;
    end;
  end;
end;

-------------------------------------------------------------------------------
-- end interface for name`'_t
-------------------------------------------------------------------------------
