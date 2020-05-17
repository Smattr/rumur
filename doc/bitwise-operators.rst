Bitwise Operators
=================
As an extension to the Murphi language, various operators for bitwise arithmetic
are supported by Rumur. These can only be used on range-typed values. Using them
with operands of any other type will result in an error.

It is generally assumed that you will use these operators on non-negative
values. There are no checks to prevent using them on negative values, but their
results are likely not what you want.

Bitwise AND and OR
------------------

.. code-block:: murphi

  x & y
  x | y

These operators are overloaded for both logical and bitwise operations. That is,
``&`` can be used for both logical AND and bitwise AND, and ``|`` can be used
for both logical OR and bitwise OR. Which interpretation is used depends on the
types of the operands. If the operands are both booleans, the operator is
interpreted as the logical form. If the operands are both range-typed, the
operator is interpreted as the bitwise form. Any other combination of operand
types is rejected as an error.

These operators have the same, potentially confusing, precedence as their C
equivalents. So you may have to sometimes use brackets to achieve the correct
effect:

.. code-block:: murphi

  var x: 0 .. 10
      y: 0 .. 10

  …

  -- brackets required here
  if (x & y) = 3 then
    …
  end;

Bitwise NOT
-----------

.. code-block:: murphi

  ~x

The number of bits used to represent range values in the verifier is variable
(controllable via command line option ``--value-type``). So you will probably
want to mask any value being NOTed with something to cap the number of bits in
the result:

.. code-block:: murphi

  var x: 0 .. 15

  …

  -- & to ensure the value of x does not exceed 4 bits
  x := ~x & 15;

Bitwise XOR
-----------

.. code-block:: murphi

  x ^ y

Left and Right Shift
--------------------

.. code-block:: murphi

  x << y
  x >> y

Unlike the C operator equivalents, left and right shifts are well-defined for
all possible shift values. A negative shift results in a shift in the opposite
direction. For example, ``x >> -3`` is equivalent to ``x << 3``. Shifting by
greater than the number of bits used to represent range values (controllable via
command line option ``--value-type``) results in ``0``. Shifting left into the
sign bit is well defined. The right shift is an arithmetic shift, not logical.
