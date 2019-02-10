/* On commit e196ed43199d6d47d36eb9f225017c2123e294c3 this model is incorrectly
 * rejected. After we fix this bug, this can be used to validate the bug has not
 * been re-introduced.
 */

var
  x: -5 .. 5;

startstate begin
  x := 1;
end;

rule begin
  x := -x;
end;
