-- This is an example of brute force Sudoku solving via model checking. It is
-- not particularly efficient, but simply demonstrates that such a thing is
-- possible.

type
  square: 1 .. 9

var
  board: array[square] of array[square] of square

-- have we finished?
function won(): boolean; begin
  return forall i: square do forall j: square do !isundefined(board[i][j]) end end;
end

-- is the value at these two positions the same
function same(i: square; j: square; k: square; l: square): boolean; begin
  return !isundefined(board[i][j]) & !isundefined(board[k][l]) & board[i][j] = board[k][l];
end

-- have we made a mistake?
function lost(): boolean; begin
  return

    -- a row contains duplicate values
    exists i: square do
      exists j: square do
        exists k: square do
          j != k & same(i, j, i, k)
        end
      end
    end
    |

    -- a column contains duplicate values
    exists i: square do
      exists j: square do
        exists k: square do
          j != k & same(j, i, k, i)
        end
      end
    end
    |

    -- a square contains duplicate values
    exists a: 0 .. 2 do
      exists b: 0 .. 2 do
        exists i: 1 .. 3 do
          exists j: 1 .. 3 do
            exists k: 1 .. 3 do
              exists l: 1 .. 3 do
                (i != k | j != l)
                  & same(a * 3 + i, b * 3 + j, a * 3 + k, b * 3 + l)
              end
            end
          end
        end
      end
    end;
end

startstate begin

  -- setup the following board:
  --
  --   ┌─┬─┬─┬─┬─┬─┬─┬─┬─┐
  --   │ │ │ │ │ │ │ │ │ │
  --   ├─┼─┼─┼─┼─┼─┼─┼─┼─┤
  --   │ │ │ │ │ │ │ │ │ │
  --   ├─┼─┼─┼─┼─┼─┼─┼─┼─┤
  --   │ │ │ │ │ │ │ │ │ │
  --   ├─┼─┼─┼─┼─┼─┼─┼─┼─┤
  --   │3│8│4│ │ │ │ │ │ │
  --   ├─┼─┼─┼─┼─┼─┼─┼─┼─┤
  --   │ │ │ │ │ │ │ │ │ │
  --   ├─┼─┼─┼─┼─┼─┼─┼─┼─┤
  --   │ │ │ │ │ │ │ │ │ │
  --   ├─┼─┼─┼─┼─┼─┼─┼─┼─┤
  --   │ │ │ │ │ │ │ │ │ │
  --   ├─┼─┼─┼─┼─┼─┼─┼─┼─┤
  --   │ │ │ │ │ │ │ │ │ │
  --   ├─┼─┼─┼─┼─┼─┼─┼─┼─┤
  --   │ │ │ │ │ │ │ │ │2│
  --   └─┴─┴─┴─┴─┴─┴─┴─┴─┘
  --
  -- This was given as an example of a hard Sudoku. I suspect it has multiple
  -- solutions, though I have not validated this.

  board[4][1] := 3;
  board[4][2] := 8;
  board[4][3] := 4;
  board[9][9] := 2;
end

-- a rule to assign an arbitrary value to a blank square
ruleset i: square; j: square do
  ruleset v: square do
    rule isundefined(board[i][j]) ==> begin
      board[i][j] := v;
    end
  end
end

-- we can stop when complete
invariant !won()

-- backtrack when we make a mistake
assume !lost()
