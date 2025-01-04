-- with GCC 11.4, this previously triggered -Wmaybe-uninitialized warnings

type
  foo: record x: 0..1 end;
  bar: record y: array[0..0] of foo; end;

var
  baz: bar;

function qux(): foo
  var w: foo;
begin
  w := baz.y[0];
  return w;
end;

function quux(): foo begin
  return qux();
end;

rule
  var z: foo;
begin
  z := quux();
  switch z.x case 0: end;
end;
