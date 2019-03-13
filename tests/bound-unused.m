-- rumur_flags: ['--bound', '10']

/* test that it's OK to set a --bound that does not have an effect because the
 * model finishes earlier than the bound
 */

var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;
