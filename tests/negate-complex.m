-- rumur_exit_code: 1

/* This tests whether we have reintroduced a bug first noticed against commit
 * e196ed43199d6d47d36eb9f225017c2123e294c3. The bug incorrectly allowed complex
 * types to appear as the argument to a negation. If this bug has been
 * reintroduced Rumur will generate code for this model that will then fail to
 * compile, when really it should reject the model from the outset.
 */

var
  x: record
    a: boolean;
  end;

startstate begin
  x.a := true;
end;

rule begin
  x.a := !x.a;
end;

rule begin
  x := -x;
end;
