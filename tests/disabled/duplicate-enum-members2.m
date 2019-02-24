-- rumur_exit_code: 1

/* A variation on duplicate-enum-members.m where we use members in different
 * types that collide.
 */

type
  t1: enum { A };
  t2: enum { A };

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;
