-- rumur_flags: ['--sandbox', 'on']
-- skip_reason: 'sandboxing unavailable on Linux prior to 3.5.0' if platform.system() == 'Linux' and tuple(int(x) for x in platform.release().split('.')) < (3, 5, 0) else None

/* Test that the sandboxing functionality is usable. Note that this merely
 * checks that the sandboxing option results in compilable code, not that the
 * sandbox itself is actually secure.
 */

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;
