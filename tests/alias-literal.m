-- test that aliasing a literal works as expected

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  alias y: true do
    if y then
      alias z: 1 do
        if z = 1 then
          x := !x;
        end;
      end;
    end;
  end;
end;
