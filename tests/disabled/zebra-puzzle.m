-- rumur_flags: ['--deadlock-detection', 'off']

/* This model is an experiment in using Rumur to solve the zebra puzzle
 * (https://en.wikipedia.org/wiki/Zebra_Puzzle). The model should eventually
 * fail the invariant with a counterexample that ends in the solution to the
 * puzzle.
 */

type
  nationality: enum { ENGLISHMAN, SPANIARD, UKRANIAN, JAPANESE, NORWEGIAN }
  colour: enum { RED, GREEN, IVORY, YELLOW, BLUE }
  animal: enum { DOG, SNAIL, FOX, HORSE, ZEBRA }
  cigarette: enum { OLDGOLD, CHESTERFIELDS, KOOLS, LUCKYSTRIKE, PARLIAMENTS }
  drink: enum { COFFEE, TEA, MILK, ORANGEJUICE, WATER }

  identity: record
    citizenship: nationality
    house_position: 0 .. 4
    paint: colour
    pet: animal
    brand: cigarette
    beverage: drink
  end

  id: scalarset(5)

var
  person: array [id] of identity

startstate begin
end

ruleset i: id do

  -- define rules to make arbitrary assignments for each person

  ruleset n: nationality do
    rule isundefined(person[i].citizenship) ==> begin
      person[i].citizenship := n;
    end
  end

  ruleset p: 0 .. 4 do
    rule isundefined(person[i].house_position) ==> begin
      person[i].house_position := p;
    end
  end

  ruleset c: colour do
    rule isundefined(person[i].paint) ==> begin
      person[i].paint := c;
    end
  end

  ruleset a: animal do
    rule isundefined(person[i].pet) ==> begin
      person[i].pet := a;
    end
  end

  ruleset c: cigarette do
    rule isundefined(person[i].brand) ==> begin
      person[i].brand := c;
    end
  end

  ruleset d: drink do
    rule isundefined(person[i].beverage) ==> begin
      person[i].beverage := d;
    end
  end

  -- define assumptions to capture the rules of the puzzle

  assume "the Englishman lives in the red house"
    !isundefined(person[i].citizenship) & !isundefined(person[i].paint)
    & person[i].citizenship = ENGLISHMAN -> person[i].paint = RED

  assume "the Spaniard owns the dog"
    !isundefined(person[i].citizenship) & !isundefined(person[i].pet)
    & person[i].citizenship = SPANIARD -> person[i].pet = DOG

  assume "coffee is drunk in the green house"
    !isundefined(person[i].paint) & !isundefined(person[i].beverage)
    & person[i].beverage = COFFEE -> person[i].paint = GREEN

  assume "the Ukranian drinks tea"
    !isundefined(person[i].citizenship) & !isundefined(person[i].beverage)
    & person[i].citizenship = UKRANIAN -> person[i].beverage = TEA

  assume "the green house is immediately to the right of the ivory house"
    forall j: id do
      !isundefined(person[i].paint) & person[i].paint = GREEN
      & !isundefined(person[i].house_position)
      & !isundefined(person[j].paint) & person[j].paint = IVORY
      & !isundefined(person[j].house_position)
      -> person[i].house_position = person[j].house_position + 1
    end

  assume "the Old Gold smoker owns snails"
    !isundefined(person[i].brand) & !isundefined(person[i].pet)
    & person[i].brand = OLDGOLD -> person[i].pet = SNAIL

  assume "Kools are smoked in the yellow house"
    !isundefined(person[i].brand) & !isundefined(person[i].paint)
    & person[i].brand = KOOLS -> person[i].paint = YELLOW

  assume "milk is drunk in the middle house"
    !isundefined(person[i].beverage) & !isundefined(person[i].house_position)
    & person[i].beverage = MILK -> person[i].house_position = 2

  assume "the Norwegian lives in the first house"
    !isundefined(person[i].citizenship) & !isundefined(person[i].house_position)
    & person[i].citizenship = NORWEGIAN -> person[i].house_position = 0

  assume "the man who smnokes Chesterfields lives in the house next to the man with the fox"
    forall j: id do
      !isundefined(person[i].brand) & !isundefined(person[j].pet)
      & !isundefined(person[i].house_position) & !isundefined(person[j].house_position)
      & person[i].brand = CHESTERFIELDS & person[j].pet = FOX
      -> person[i].house_position = person[j].house_position + 1
       | person[j].house_position = person[i].house_position + 1
    end

  assume "Kools are smoked in the house next to the house where the horse is kept"
    forall j: id do
      !isundefined(person[i].brand) & !isundefined(person[j].pet)
      & !isundefined(person[i].house_position) & !isundefined(person[j].house_position)
      & person[i].brand = KOOLS & person[j].pet = HORSE
      -> person[i].house_position = person[j].house_position + 1
       | person[j].house_position = person[i].house_position + 1
    end

  assume "the Lucky Strike smoker drinks orange juice"
    !isundefined(person[i].brand) & !isundefined(person[i].beverage)
    & person[i].brand = LUCKYSTRIKE -> person[i].beverage = ORANGEJUICE

  assume "the Japanese smokes Parliaments"
    !isundefined(person[i].citizenship) & !isundefined(person[i].brand)
    & person[i].citizenship = JAPANESE -> person[i].brand = PARLIAMENTS

  assume "the Norwegian lives next to the blue house"
    forall j: id do
      !isundefined(person[i].citizenship) & !isundefined(person[j].paint)
      & !isundefined(person[i].house_position) & !isundefined(person[j].house_position)
      & person[i].citizenship = NORWEGIAN & person[j].paint = BLUE
      -> person[i].house_position = person[j].house_position + 1
       | person[j].house_position = person[i].house_position  + 1
    end

end

-- terminate as soon as everything in the model has been chosen
invariant
  exists i: id do isundefined(person[i].citizenship)
                | isundefined(person[i].house_position)
                | isundefined(person[i].paint)
                | isundefined(person[i].pet)
                | isundefined(person[i].brand)
                | isundefined(person[i].beverage)
               end
