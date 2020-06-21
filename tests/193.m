-- This model is a stripped back version of a test case that produced a checker
-- assertion failure on commit 4944427734628cf913e8d5eeb54d897033f9eb59. If
-- caused record fields to be reordered in a way such that they were no longer
-- aligned with references to those fields. If this bug has been introduced,
-- this will cause an assertion failure during checking when the verifier is
-- generated with --debug.
--
-- See https://github.com/Smattr/rumur/issues/193

type t : record
  a   : boolean;
  b  : boolean;
  c    : boolean;
endrecord;

type t2 : scalarset(4);

var p  : array [ t2 ] of t;

var q  : record
  x     : boolean;
  y : t2;
  z  : boolean;
endrecord;

startstate
  q.x    := false;
  for ix : t2 do p[ix].a := false; endfor;
endstartstate

ruleset
  r : t2
do
  rule
    !q.x & p[r].a & !p[r].c
  ==>
    q.y  := r;
    q.x      := true;
  endrule

  rule
    !p[r].a & !q.x
  ==>
    p[r].a := true;
    p[r].c := false;
  endrule
endruleset

rule
  q.x
==>
  q.y := q.y;
  q.x := false;
endrule
