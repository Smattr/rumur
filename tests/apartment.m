/* This model represents a situation I had where a friend was staying with me
 * and I had lent them one of my keys to get in. It's primarily used as a test
 * of the 'liveness' keyword. However, a close reading can probably also reveal
 * the secrets to robbing me.
 */

type
  location: enum {
    APARTMENT, -- inside the apartment
    ELEVATOR,  -- the building elevator
    GARAGE,    -- in the underground garage
    HALLWAY,   -- the hallway on my floor
    LOBBY,     -- the lobby of the building
    STAIRWELL, -- in the building stairwell
    STREET,    -- in the street outside
  }

  person: enum { ME, FRIEND }

var
  -- the location of myself and my friend
  people: array[person] of location

startstate begin

  -- I start in my apartment
  people[ME] := APARTMENT;

  -- my friend is outside
  people[FRIEND] := STREET;

end

ruleset p: person do

  -- anyone can leave if they're in the apartment
  rule "exit apartment" people[p] = APARTMENT ==> begin
    people[p] := HALLWAY;
  end

  /* either of us can get into the apartment from the hallway because we both
   * have a key
   */
  rule "enter apartment" people[p] = HALLWAY ==> begin
    people[p] := APARTMENT;
  end

  -- the elevator has no security, except in the garage
  rule "enter elevator" people[p] = HALLWAY | people[p] = LOBBY ==> begin
    people[p] := ELEVATOR;
  end
  rule "exit elevator to hallway" people[p] = ELEVATOR ==> begin
    people[p] := HALLWAY;
  end
  rule "exit elevator to lobby" people[p] = ELEVATOR ==> begin
    people[p] := LOBBY;
  end
  rule "exit elevator to garage" people[p] = ELEVATOR ==> begin
    people[p] := GARAGE;
  end

  -- the stairwell has no security to get in
  -- note: not possible to enter the stairwell from the street
  rule "enter stairwell"
      people[p] = HALLWAY | people[p] = LOBBY | people[p] = GARAGE ==> begin
    people[p] := STAIRWELL;
  end

  /* the stairwell only requires a key to get out on my level or the lobby, but
   * it's the same key both of us have a copy of
   */
  rule "exit stairwell to street" people[p] = STAIRWELL ==> begin
    people[p] := STREET;
  end
  rule "exit stairwell to garage" people[p] = STAIRWELL ==> begin
    people[p] := GARAGE;
  end
  rule "exit stairwell to lobby" people[p] = STAIRWELL ==> begin
    people[p] := LOBBY;
  end
  rule "exit stairwell to hallway" people[p] = STAIRWELL ==> begin
    people[p] := HALLWAY;
  end

  -- you can exit the lobby to the street with no key
  rule "exit lobby to street" people[p] = LOBBY ==> begin
    people[p] := STREET;
  end

  -- anyone can exit the garage onto the street
  rule "exit garage to street" people[p] = GARAGE ==> begin
    people[p] := STREET;
  end

end

/* you can only enter the lobby from the street with the door fob, which my
 * friend has
 */
rule "enter lobby from street" people[FRIEND] = STREET ==> begin
  people[FRIEND] := LOBBY;
end

-- my friend can also let me in if I'm outside
rule "enter lobby from street with me"
    people[FRIEND] = STREET & people[ME] = STREET ==> begin
  people[FRIEND] := LOBBY;
  people[ME] := LOBBY;
end

-- you can only enter the garage from the street with the remote, which I have
rule "enter garage from street" people[ME] = STREET ==> begin
  people[ME] := GARAGE;
end

-- I can let my friend in if he's with me
rule "enter garage from street with friend"
    people[ME] = STREET & people[FRIEND] = STREET ==> begin
  people[ME] := GARAGE;
  people[FRIEND] := GARAGE;
end

/* getting into the elevator from the garage for some reason requires the door
 * fob, which my friend has
 */
rule "enter elevator from garage" people[FRIEND] = GARAGE ==> begin
  people[FRIEND] := ELEVATOR;
end

-- if we're together, my friend can swipe me in
rule "enter elevator from garage together"
    people[FRIEND] = GARAGE & people[ME] = GARAGE ==> begin
  people[FRIEND] := ELEVATOR;
  people[ME] := ELEVATOR;
end

-- regardless of where we are, I should be able to get back into my apartment
liveness "I can get in" people[ME] = APARTMENT

-- similarly, my friend should always be able to get in
liveness "my friend can get in" people[FRIEND] = APARTMENT
