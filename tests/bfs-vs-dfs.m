-- checker_exit_code: 1

/* A model to explore Breadth-First Search vs Depth-First Search as exploration
 * strategies. This model contains multiple errors, different ones of which will
 * be found depending on which search strategy you use.
 */

var
  x: boolean;
  y: boolean;
  z: boolean;
  w: boolean;

startstate begin
  x := false;
  y := false;
  z := false;
  w := false;
end;

rule "A" !x ==> begin
  x := true;
end;

rule "B" !x ==> begin
  y := true;
end;

rule "C" y ==> begin
  z := true;
end;

rule "D" z ==> begin
  w := true;
end;

rule "E" x ==> begin
  w := true;
end;

invariant !w
