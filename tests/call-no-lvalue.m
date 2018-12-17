/* This model tries several variations on passing a non-lvalue as a var
 * parameter. It's not clear whether this should be accepted or not, but it
 * seems obvious what the user meant if they wrote this, so we aim to accept it.
 * On commit fe9344f5b723608cd8916bd16c2688f9494ca92a, this causes an uncaught
 * exception.
 */

var
  x: boolean;

function foo(var y: 0 .. 100): boolean; begin
  assert y = 42;
  return true;
end;

function bar(var z: boolean): boolean; begin
  assert z;
  return true;
end;

procedure baz(var w: 0 .. 100); begin
  assert w = 42;
end;

procedure qux(var v: boolean); begin
  assert v;
end;

startstate begin
  x := true;
end;

rule begin
  foo(42);
  bar(true);
  baz(42);
  qux(true);
  x := !x;
end;
