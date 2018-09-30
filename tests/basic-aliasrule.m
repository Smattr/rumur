var
  x: boolean;

startstate begin
  x := true;
end;

alias y: x do
  rule begin
    y := !y;
  end;
end;
