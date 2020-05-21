-- test that Rumur extension bitwise NOT is accepted

var
  x: 0 .. 10
  y: boolean

startstate begin
  y := true;
end

rule begin

  -- we cannot guarantee any particular value_t, so need to be a bit fuzzy with
  -- our assertions

  x := 0;
  assert ~~x = x;
  x := 1;
  assert ~~x = x;
  x := 2;
  assert ~~x = x;

  assert ~x != 0 & ~x != 2;

  y := !y;
end
