-- Test of all sorts of alias statement variations.

var
  x: boolean;
  y: record z: boolean end;

procedure foo(a: boolean; var b: boolean); begin

  -- Alias of a read-only parameter
  alias w: a do
    b := !w;
  end;

  -- Alias of a writable parameter
  alias w: b do
    w := !w;
  end;

end;

-- As above for a function, just to make sure there's nothing odd there
function bar(a: boolean; var b: boolean): boolean begin

  alias w: a do
    b := !w;
  end;

  alias w: b do
    w := !w;
  end;

  -- Let's try returning an alias too just for kicks
  alias w: a do
    return w;
  end;

  -- Unreachable, but test generation of returning an lvalue-eligible alias
  alias w: b do
    return w;
  end;

end;

startstate begin
  x := true;
  y.z := true;
end;

rule begin

  -- Alias of a state variable
  alias w: x do
    w := !w;
  end;

  -- Alias of a complex state variable
  alias w: y do
    w.z := !w.z;
  end;

  -- Alias of a non-top-level state variable
  alias w: y.z do
    w := !w;
  end;

end;

ruleset c: boolean do
  rule begin

    -- Alias a ruleset parameter
    alias w: c do
      x := w;
    end;

  end;
end;
