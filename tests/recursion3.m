-- rumur_exit_code: 1

-- test that mutual recursion is rejected

var
  x: boolean;

function is_even(n: 0 .. 100): boolean; begin
  if n = 0 then
    return true;
  else
    return is_odd(n - 1);
  end;
end;

function is_odd(n: 0 .. 100): boolean; begin
  if n = 0 then
    return false;
  else
    return is_even(n - 1);
  end;
end;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;
