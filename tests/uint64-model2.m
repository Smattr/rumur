-- rumur_exit_code: 1

/* A variant of uint64-model.m that tests for a mistake I first made wherein the
 * `--value-type auto` logic did not account for the fact that we need an extra
 * value to store `undefined`. I.e. If we only have types up to uint64_t, we
 * cannot represent a full 64-bit range.
 */

var
  x: 0 .. 0xffffffffffffffff;

startstate begin
  x := 0;
end;

rule begin
  x := 0xffffffffffffffff - x;
end;
