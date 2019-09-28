/* This model attempts to provoke a bug first observed on
 * b1e28bfc4fe1b042f7e3034a1516cd20df789b51. The issue was that negative
 * literals are considered as the negation of a positive literal and Rumur would
 * only look at the inner (positive) literal when determining the value type
 * (value_t). As a result, for the following model it would incorrectly choose
 * the value type uint8_t that cannot contain -1. If this bug has been
 * reintroduced, this model will error with a subtraction overflow.
 */

var
  x: 0 .. 9;

startstate begin
  x := 0;
end;

rule x > 0 ==> begin
  x := x + -1;
end;

rule x < 9 ==> begin
  x := x + 1;
end;
