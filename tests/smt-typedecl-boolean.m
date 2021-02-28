-- rumur_flags: CONFIG['SMT_ARGS']
-- skip_reason: 'no SMT solver available' if CONFIG['SMT_ARGS'] is None else None

-- This model is designed to provoke a problem first observed on
-- 787f074328874a470d595576ae9e8b16837582f4, where the SMT bridge would
-- incorrectly mangle 'boolean' as part of a TypeDecl. This would lead to SMT
-- problems including the undefined symbol 'ru_boolean'.

type
  bit: boolean;

var
  x: bit;
  y: bit;

startstate begin
  x := true;
end;

rule begin
  -- if the SMT bridge correctly understands the problem passed to it, it should
  -- detect the following as a tautology causing simplification to collapse it
  -- to 'true' avoid the read of an undefined variable
  if y = true | y = false then
    x := !x;
  end;
end;
