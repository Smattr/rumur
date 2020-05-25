-- rumur_flags: ['--deadlock-detection', 'off']

/* This model is an experiment in using Rumur to solve the zebra puzzle
 * (https://en.wikipedia.org/wiki/Zebra_Puzzle). The model should eventually
 * fail the invariant with a counterexample that ends in the solution to the
 * puzzle.
 */

type
  nationality: enum { ENGLISHMAN, SPANIARD, UKRAINIAN, JAPANESE, NORWEGIAN }
  colour: enum { RED, GREEN, IVORY, YELLOW, BLUE }
  animal: enum { DOG, SNAIL, FOX, HORSE, ZEBRA }
  cigarette: enum { OLDGOLD, CHESTERFIELDS, KOOLS, LUCKYSTRIKE, PARLIAMENTS }
  drink: enum { COFFEE, TEA, MILK, ORANGEJUICE, WATER }

  house: record
    citizenship: nationality
    paint: colour
    pet: animal
    brand: cigarette
    beverage: drink
  end

var
  houses: array [0 .. 4] of house

startstate begin
end

ruleset i: 0 .. 4 do

  -- define rules to make arbitrary assignments for each house

  ruleset n: nationality do
    rule isundefined(houses[i].citizenship) ==> begin
      houses[i].citizenship := n;
    end
  end

  ruleset c: colour do
    rule isundefined(houses[i].paint) ==> begin
      houses[i].paint := c;
    end
  end

  ruleset a: animal do
    rule isundefined(houses[i].pet) ==> begin
      houses[i].pet := a;
    end
  end

  ruleset c: cigarette do
    rule isundefined(houses[i].brand) ==> begin
      houses[i].brand := c;
    end
  end

  ruleset d: drink do
    rule isundefined(houses[i].beverage) ==> begin
      houses[i].beverage := d;
    end
  end

  -- define assumptions to capture the rules of the puzzle

  assume "the Englishman lives in the red house"
    !isundefined(houses[i].citizenship) & !isundefined(houses[i].paint)
    & houses[i].citizenship = ENGLISHMAN -> houses[i].paint = RED

  assume "the Spaniard owns the dog"
    !isundefined(houses[i].citizenship) & !isundefined(houses[i].pet)
    & houses[i].citizenship = SPANIARD -> houses[i].pet = DOG

  assume "coffee is drunk in the green house"
    !isundefined(houses[i].paint) & !isundefined(houses[i].beverage)
    & houses[i].beverage = COFFEE -> houses[i].paint = GREEN

  assume "the Ukrainian drinks tea"
    !isundefined(houses[i].citizenship) & !isundefined(houses[i].beverage)
    & houses[i].citizenship = UKRAINIAN -> houses[i].beverage = TEA

  assume "the green house is immediately to the right of the ivory house"
    forall j: 0 .. 4 do
      !isundefined(houses[i].paint) & houses[i].paint = GREEN
      & !isundefined(houses[j].paint) & houses[j].paint = IVORY
      -> i = j + 1
    end

  assume "the Old Gold smoker owns snails"
    !isundefined(houses[i].brand) & !isundefined(houses[i].pet)
    & houses[i].brand = OLDGOLD -> houses[i].pet = SNAIL

  assume "Kools are smoked in the yellow house"
    !isundefined(houses[i].brand) & !isundefined(houses[i].paint)
    & houses[i].brand = KOOLS -> houses[i].paint = YELLOW

  assume "milk is drunk in the middle house"
    !isundefined(houses[i].beverage)
    & houses[i].beverage = MILK -> i = 2

  assume "the Norwegian lives in the first house"
    !isundefined(houses[i].citizenship)
    & houses[i].citizenship = NORWEGIAN -> i = 0

  assume "the man who smnokes Chesterfields lives in the house next to the man with the fox"
    forall j: 0 .. 4 do
      !isundefined(houses[i].brand) & !isundefined(houses[j].pet)
      & houses[i].brand = CHESTERFIELDS & houses[j].pet = FOX
      -> i = j + 1 | j = i + 1
    end

  assume "Kools are smoked in the house next to the house where the horse is kept"
    forall j: 0 .. 4 do
      !isundefined(houses[i].brand) & !isundefined(houses[j].pet)
      & houses[i].brand = KOOLS & houses[j].pet = HORSE
      -> i = j + 1 | j = i + 1
    end

  assume "the Lucky Strike smoker drinks orange juice"
    !isundefined(houses[i].brand) & !isundefined(houses[i].beverage)
    & houses[i].brand = LUCKYSTRIKE -> houses[i].beverage = ORANGEJUICE

  assume "the Japanese smokes Parliaments"
    !isundefined(houses[i].citizenship) & !isundefined(houses[i].brand)
    & houses[i].citizenship = JAPANESE -> houses[i].brand = PARLIAMENTS

  assume "the Norwegian lives next to the blue house"
    forall j: 0 .. 4 do
      !isundefined(houses[i].citizenship) & !isundefined(houses[j].paint)
      & houses[i].citizenship = NORWEGIAN & houses[j].paint = BLUE
      -> i = j + 1 | j = i + 1
    end

end

-- terminate as soon as everything in the model has been chosen
invariant
  exists i: 0 .. 4 do isundefined(houses[i].citizenship)
                    | isundefined(houses[i].paint)
                    | isundefined(houses[i].pet)
                    | isundefined(houses[i].brand)
                    | isundefined(houses[i].beverage)
                   end
