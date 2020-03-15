-- This model tries to provoke a bug first seen in commit
-- 7f31193bb5749fd267b1049aefa40efef98db33f, wherein enum-valued constants would
-- be given an incorrect type in the murphi2c output. If the bug has been
-- reintroduced murphi2c will emit the constant `a` with the type `t2` instead
-- of `t1`.

type
  t1: enum { A, B };
  t2: enum { X, Y, Z };

const
  a: A;

var
  x: boolean;

startstate begin
  x := false;
end;

rule begin
  x := !x;
end;
