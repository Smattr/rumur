-- rumur_flags: ['--sandbox', 'on']
-- skip_reason: None if CONFIG['HAS_SANDBOX'] else 'no suitable sandboxing facilities available on this platform'

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
