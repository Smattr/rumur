-- rumur_exit_code: 1

-- Test that switching on complex type is rejected

var
  x: record
    y: boolean;
  end;

startstate begin
  x.y := true;
end;

rule begin

  switch x
    -- There's nothing reasonable we can write as the cases here, but stress the
    -- validation logic even more by giving it no cases that it could detect as
    -- something to reject.
  end;

end;
