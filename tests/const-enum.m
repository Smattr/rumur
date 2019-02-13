-- Various unusual usages of a const with enum type

type
  t: enum { A, B, C };

const
  X: A;

var
  x: t;

startstate begin
  x := A;
end;

rule begin
  x := B;
end;

rule begin
  x := X;
end;

rule begin
  if x = X then
    x := C;
  end;
end;

rule begin
  switch X
    case A:
      x := B;
  end;
end;

rule begin
  switch X
    case X:
      x := X;
  end;
end;
