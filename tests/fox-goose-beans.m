-- rumur_flags: ['--deadlock-detection', 'off']
-- checker_exit_code: 1

/* This is a model of the "Fox, goose and bag of beans puzzle"
 * https://en.wikipedia.org/wiki/Fox,_goose_and_bag_of_beans_puzzle
 *
 * We represent the location of each of the parties (fox, goose, beans and human
 * chaperone) as either side of the river (east or west) or in the boat itself.
 * All parties start on the east side of the river. The object is to get all
 * parties to the west side of the river. The boat can only carry the human by
 * themselves or with one other party. All of this we encode as transition
 * rules.
 *
 * The two failure cases (the fox eats the goose when left alone with it and the
 * goose eats the beans when left alone with them) are encoded as assumptions.
 * The *negation* of the goal itself is encoded as an invariant. Thus what we
 * should find is that this model *fails*, producing a counterexample trace that
 * gives a solution to the puzzle.
 *
 * One such solution is:
 *  1. the human crosses to west with the goose
 *  2. the human crosses back east with nothing
 *  3. the human crosses west with the beans
 *  4. the human crosses back east with the goose
 *  5. the human crosses west with the fox
 *  6. the human crosses back east with nothing
 *  7. the human crosses west with goose
 */

type
  location: enum { EAST, BOAT, WEST };

var
  fox: location;
  goose: location;
  beans: location;
  human: location;

startstate begin
  fox := EAST;
  goose := EAST;
  beans := EAST;
  human := EAST;
end;

rule "set out with fox"
  fox = human & human != BOAT ==>
begin
  human := BOAT;
  fox := BOAT;
end;

rule "set out with goose"
  goose = human & human != BOAT ==>
begin
  human := BOAT;
  goose := BOAT;
end;

rule "set out with beans"
  beans = human & human != BOAT ==>
begin
  human := BOAT;
  beans := BOAT;
end;

rule "set out alone"
  human != BOAT ==>
begin
  human := BOAT;
end;

rule "arrive east"
  human = BOAT ==>
begin
  human := EAST;
  if fox = BOAT then
    fox := EAST;
  end;
  if goose = BOAT then
    goose := EAST;
  end;
  if beans = BOAT then
    beans := EAST;
  end;
end;

rule "arrive west"
  human = BOAT ==>
begin
  human := WEST;
  if fox = BOAT then
    fox := WEST;
  end;
  if goose = BOAT then
    goose := WEST;
  end;
  if beans = BOAT then
    beans := WEST;
  end;
end;

assume "fox prevented from eating goose"
  fox = goose -> human = fox;

assume "goose prevented from eating beans"
  goose = beans -> human = goose;

invariant "goal"
  fox != WEST | goose != WEST | beans != WEST | human != WEST;
