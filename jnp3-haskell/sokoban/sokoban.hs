{-# LANGUAGE OverloadedStrings #-}

import CodeWorld
import qualified Data.Text as Text

data Tile = Wall | Ground | Storage | Box | Blank deriving Eq
data Direction = R | U | L | D deriving Eq
data Coord = C Integer Integer deriving (Show, Eq)
data SSState world = StartScreen | Running world deriving Eq
data Activity world = Activity {
    actState  :: world,
    actHandle :: (Event -> world -> world),
    actDraw   ::(world -> Picture)
    }
data State = State {
  playerCoord     :: Coord,
  playerDirection :: Direction,
  boxesCoords     :: [Coord]
  } deriving Eq

initialCoord :: Coord
initialCoord = C 0 0

atCoord :: Coord -> Picture -> Picture
atCoord (C x y) pic = translated (fromIntegral x) (fromIntegral y) pic

adjacentCoord :: Direction -> Coord -> Coord
adjacentCoord R (C x y) = C (x + 1) y
adjacentCoord U (C x y) = C x (y + 1)
adjacentCoord L (C x y) = C (x - 1) y
adjacentCoord D (C x y) = C x (y - 1)

moveCoords :: [Direction] -> Coord -> Coord
moveCoords [] (C x y) = (C x y)
moveCoords (l:ls) (C x y) = adjacentCoord l (moveCoords ls (C x y))

wall, ground, storage, box :: Picture

wall = colored gray (solidRectangle 1 1)
ground = colored (light orange) (solidRectangle 1 1)
storage = colored pink (solidCircle 0.4) & ground
box = pictures([translated (x * 0.2 - 0.4) 0 (rectangle 0.2 1) | x <- [0..4]] 
      ++ [colored brown (solidRectangle 1 1)])
      
      
drawTile :: Tile -> Picture
drawTile Wall    = wall
drawTile Ground  = ground
drawTile Storage = storage
drawTile Box     = box
drawTile Blank   = blank

player :: Picture
player = (solidRectangle 0.2 1) 
        & translated 0.1 0.3 ((rotated (pi / 4) (solidRectangle 0.2 0.5)))
        & translated (-0.1) 0.3 ((rotated (-pi / 4) (solidRectangle 0.2 0.5)))

player2 :: Direction -> Picture
player2 U = rotated 0 player
player2 R = rotated (-pi / 2) player
player2 D = rotated (-pi) player
player2 L = rotated (-3 * pi / 2) player


-- Moim zdaniem najsensowniejszym rozwiązaniem problemu
-- puszczenia przycisku Esc jest ignorowanie tego zdarzenia.
-- Zdenerwowany i spieszący się gracz być może zacząłby ruch
-- przed puszczeniem przycisku Esc, a gdybyśmy wówczas 
-- znowu zresetowali grę to zirytowałoby go to jeszcze bardziej.
resetState :: 
    world -> 
    (Event -> world -> world) -> 
    Event -> 
    world -> 
    world
resetState initState stateFun event@(KeyPress key) state
    | key == "Esc"  = initState
    | otherwise     = stateFun event state
resetState _ stateFun event state = stateFun event state

startScreen :: Picture
startScreen = etap4

endScreen :: Picture
endScreen = lettering "You win! Press ESC to play again."

resettable :: Activity s -> Activity s
resettable (Activity state0 handle draw)
  = Activity state0 handle' draw
  where handle' (KeyPress key) _ | key == "Esc" = state0
        handle' e s = handle e s
        
withStartScreen :: Activity s -> Activity (SSState s)
withStartScreen (Activity state0 handle draw)
  = Activity state0' handle' draw'
  where
    state0' = StartScreen

    handle' (KeyPress key) StartScreen
         | key == " "                  = Running state0
    handle' _              StartScreen = StartScreen
    handle' e              (Running s) = Running (handle e s)

    draw' StartScreen = startScreen
    draw' (Running s) = draw s
    
data WithUndo a = WithUndo a [a]

withUndo :: Eq a => Activity a -> Activity (WithUndo a)
withUndo (Activity state0 handle draw) = Activity state0' handle' draw' where
    state0' = WithUndo state0 []
    handle' (KeyPress key) (WithUndo s stack) | key == "U"
      = case stack of s':stack' -> WithUndo s' stack'
                      []        -> WithUndo s []
    handle' e              (WithUndo s stack)
       | s' == s = WithUndo s stack
       | otherwise = WithUndo (handle e s) (s:stack)
      where s' = handle e s
    draw' (WithUndo s _) = draw s
    
    
runActivity :: Eq s => Activity s -> IO ()
runActivity a = activityOf state0 handle draw
  where (Activity state0 handle draw) = withUndo (withStartScreen (resettable a))


data Maze = Maze Coord (Coord -> Tile) 

removeBoxes :: Maze -> Maze
removeBoxes (Maze coord maze) = (Maze coord (f . maze)) 
  where 
    f Box  = Ground
    f x    = x

  
addBoxes :: [Coord] -> Maze -> Maze
addBoxes boxes (Maze coord maze) = (Maze coord newMaze)
  where 
    newMaze x 
      | x `elem` boxes = Box
      | otherwise      = maze x
      
sampleMaze :: Maze
sampleMaze = (Maze (C 0 (-1)) (
  \(C x y) ->
    if abs x > 4  || abs y > 4       then Blank
    else if abs x == 4 || abs y == 4 then Wall
    else if x ==  2 && y <= 0        then Wall
    else if x ==  3 && y <= 0        then Storage
    else if x >= -2 && y == 0        then Box
    else                                  Ground))
  
emptyMaze = removeBoxes sampleMaze
        
      
drawBoard :: State -> Picture
drawBoard (State (C px py) dir boxes) = 
  translated (fromInteger px) (fromInteger py) (player2 dir) 
  & pictures([translated x y (drawTile (maze (C (round x) (round y)))) | x <- [-10..10], y <- [-10..10]])
  where
    (Maze _ maze) = addBoxes boxes emptyMaze

draw :: State -> Picture
draw state
  | isWinning state = endScreen
  | otherwise       = drawBoard state
  
    
isEmptyTileWalkable :: Coord -> Bool
isEmptyTileWalkable coord
    | tile == Ground || tile == Storage = True
    | otherwise                         = False
    where 
       (Maze _ empty) = emptyMaze
       tile = empty coord
    
-- Uwaga! Obecnie nie ma pozwolenia na przesuwanie wielu skrzyń na raz.
isNextStatePossible :: State -> Bool
isNextStatePossible (State coord dir boxes)
  | coord `elem` boxes = isEmptyTileWalkable behindBox && (not (behindBox `elem` boxes))
  | otherwise          = isEmptyTileWalkable coord
  where
    behindBox = adjacentCoord dir coord
    
moveBox :: State -> [Coord]
moveBox (State coord dir boxes) = map (\c -> if c == coord then (adjacentCoord dir coord) else c) boxes

nextState :: State -> Direction -> State
nextState (State pCoord _ boxes) dir = 
    if (isNextStatePossible tempNextState) 
    then (State newCoord dir (moveBox tempNextState))
    else (State pCoord dir boxes)
    where 
      newCoord = adjacentCoord dir pCoord
      tempNextState = (State newCoord dir boxes)
      

isWinning :: State -> Bool
isWinning (State _ _ boxes)= andList (map (\c -> empty c == Storage) boxes)
    where (Maze _ empty) = emptyMaze

-- Polecenie mówiło, że mamy reagować tylko na strzałki, 
-- ale moim zdaniem dodanie WASD nie jest złe.
handleEvent :: Event -> State -> State
handleEvent (KeyPress key) state
    | isWinning state              = state
    | key == "Right" || key == "D" = go R
    | key == "Up"    || key == "W" = go U
    | key == "Left"  || key == "A" = go L
    | key == "Down"  || key == "S" = go D
     where 
       go d = (nextState state d)
handleEvent _ c = c
    
initialState :: Maze -> State
initialState (Maze initialCoord maze) = State initialCoord U initialBoxes
    where initialBoxes = [(C x y) | x <- [-10..10], y <- [-10..10], maze (C x y) == Box]
    
foldList :: (a -> b -> b) -> b -> [a] -> b
foldList _ iter [] = iter
foldList func iter (x:xs) = foldList func (func x iter) xs
    
elemList :: Eq a => a -> [a] -> Bool
elemList element list = foldList func False list
    where func front val = if element == front then True else False || val

appendList :: [a] -> [a] -> [a]
appendList first second = foldList func first second
    where func element list = list ++ [element]

listLength :: [a] -> Integer
listLength list = foldList func 0 list
    where func _ count = count + 1

filterList :: (a -> Bool) -> [a] -> [a]
filterList cond list = foldList func [] list
    where func element approved = if (cond element) then approved ++ [element] else approved

nth :: [a] -> Integer -> a
nth list n = found 
    where 
      func element (curr, i) = if i > n then (curr, i + 1) else (element, i + 1)
      (found, _) = foldList func (x, 1) list
      (x:xs) = list

mapList :: (a -> b) -> [a] -> [b]
mapList mapping list = foldList func [] list
    where func element list = list ++ [mapping element]

andList :: [Bool] -> Bool
andList list = foldList func True list
    where func element val = element && val

allList :: (a-> Bool) -> [a] -> Bool
allList cond list = foldList func True list
    where func element val = (cond element) && val
    
getAllReachable :: Eq a => a -> [a] -> (a -> [a]) -> [a]
getAllReachable vertex seen neighbours = foldList traversal updatedSeen notSeenNeighs
    where 
      notSeenNeighs = filterList (\x -> not (elemList x seen)) (neighbours vertex)
      updatedSeen = appendList seen notSeenNeighs
      traversal newVertex newSeen = getAllReachable newVertex newSeen neighbours
    
isGraphClosed :: Eq a => a -> (a -> [a]) -> (a -> Bool) -> Bool
isGraphClosed initial neighbours isOk = allList isOk reachableVertices
    where reachableVertices = getAllReachable initial [initial] neighbours
    
-- Naiwne, ale działa.
reachable :: Eq a => a -> a -> (a -> [a]) -> Bool
reachable v initial neighbours = elemList v (getAllReachable initial [initial] neighbours)

-- Jeszcze bardziej naiwne, ale też działa.
-- Alternatywnie, znajdźmy wszystkie osiągalne wierzchołki i dla każdego w vs sprawdźmy, czy są w tej liście (kapkę lepsze):
-- allReachable vs initial neighbours = allList newReachable vs
--     where 
--         reachables = getAllReachable initial [initial] neighbours
--         newReachable v = elemList v reachables
allReachable :: Eq a => [a] -> a -> (a -> [a]) -> Bool
allReachable vs initial neighbours = allList newReachable vs
    where newReachable v = reachable v initial neighbours
    
    
-- Być może źle zrozumiałem definicję "osiągalności".
isWalkable :: Tile -> Bool
isWalkable tile = tile == Ground || tile == Storage || tile == Box
    
tileNeighbours :: Maze -> Coord -> [Coord]
tileNeighbours (Maze _ maze) (C x y) = [coord | coord <- [(C (x + 1) y), (C (x - 1) y), (C x (y + 1)), (C x (y - 1))], isWalkable (maze coord)]
    
isClosed :: Maze -> Bool
isClosed (Maze start maze) = startingPointOk && (isGraphClosed start (tileNeighbours (Maze start maze)) isOk)
    where
      (C x y) = start
      startingPointOk = isWalkable (maze start)
      neighbours = [coord | coord <- [(C (x + 1) y), (C (x - 1) y), (C x (y + 1)), (C x (y - 1))], isWalkable (maze coord)]
      isOk coord = (maze coord) /= Blank

isSane :: Maze -> Bool
isSane (Maze start maze) = (listLength saneBoxes) <= (listLength saneStorages)
    where
      reachables = getAllReachable start [start] (tileNeighbours (Maze start maze))
      saneBoxes = filterList (\x -> maze x == Box) reachables
      saneStorages = filterList (\x -> maze x == Storage) reachables

-- "Moje" (pożyczone z internetu, lecz zaimplementowane przeze mnie) plansze:
-- Źródło: http://www.sneezingtiger.com/sokoban/levelpics/microcosmosImages.html
-- Zaszła zmiana, ponieważ pozycją startową dla pudełka było pole Storage, czego scenariusz labu nie przewiduje.
microcosmos01 :: Maze -- działa, rozwiązywalne
microcosmos01 = Maze (C 0 0) (
       \(C x y) ->
         if y == 1 && abs x == 2                   then Box
         else if x == 1 && y == -1                 then Box  
         else if x == 0 && y > -3 && y < 1         then Storage
         else if abs x > 4 || abs y > 3            then Blank
         else if x == 0 && y == 3                  then Blank
         else if abs x > 2 && y < -1               then Blank
         else if abs x == 1 && y > -3 && y < 2     then Ground
         else if x == 0 && y == 1                  then Ground
         else if (abs x > 1 && abs x < 4) 
             && (y > -1 && y < 3)                   then Ground
         else                                           Wall)

-- Autorskie plansze
unsolvableSquare :: Maze -- nierozwiązywalne
unsolvableSquare = Maze (C 0 0) (
        \(C x y) ->
          if x == -1 && y == -1 then Box
          else if x == 1 && y == 1 then Storage
          else if (x == 1 && y == 0) || (x == 0 && y == 1) then Wall
          else if abs x == 2 && abs y < 3 then Wall
          else if abs y == 2 && abs x < 3 then Wall
          else if abs x < 2 && abs y < 2 then Ground
          else                                 Blank)

-- Plansze od innych studentów:
mazeBK :: Maze
mazeBK = Maze (C (-1) 0) (
       \(C x y) ->
         if abs x > 2 || abs y > 1 then Blank
         else if x == -1 && y == 0 then Ground
         else if x ==  0 && y == 0 then Box
         else if x ==  1 && y == 0 then Storage
         else                           Wall)
         
mazeBig :: Maze
mazeBig = Maze (C 0 1) foo 
  where foo :: Coord -> Tile
        foo (C x y)
          | abs x > 6 || abs y > 4 = Blank
          | abs x == 6 || abs y == 4 = Wall
          | x == 2 && y <= 0 = Wall
          | x == 1 && y <= -2 = Storage
          | x == -1 && y <= -2 = Storage
          | x == -3 && y <= -2 = Storage
          | x >= -3 && y == 0 && x <= 3 = Box
          | otherwise = Ground
          
tooManyBoxes :: Maze --nierozwiązywalne
tooManyBoxes = Maze (C 0 0) maze
    where maze (C x y)
            | abs x > 4  || abs y > 2         = Blank
            | x >= -1 && x <= 4 && abs y == 1 = Wall
            | x == 4 && y == 0                = Wall
            | x == -1 && y == 0               = Wall
            | x ==  0 && y == 0               = Ground
            | (x == 1 || x == 2) && y == 0    = Box
            | x == 3 && y == 0                = Storage
            | otherwise                       = Blank
              
snakeMaze :: Maze
snakeMaze = Maze (C 0 (-1)) maze
   where maze (C x y)
            | abs x > 3  || abs y > 3    = Blank
            | abs x == 4 || abs y == 4   = Wall
            | x ==  -1 && y >= -1        = Wall
            | x ==  1 && y <= -1         = Wall
            | x == -2 && y == 0          = Storage
            | x == 0 && y == -2          = Storage
            | (x == 0 || x == 1)  && y == 0   = Box
            | otherwise                  = Ground

mazes :: [Maze]
mazes = [mazeBK, sampleMaze, mazeBig, microcosmos01, snakeMaze]

badMazes :: [Maze]
badMazes = [unsolvableSquare, tooManyBoxes]

pictureOfBools :: [Bool] -> Picture
pictureOfBools xs = translated (-fromIntegral k /2) (fromIntegral k) (go 0 xs)
  where n = length xs
        k = findK 0 -- k is the integer square of n
        findK i | i * i >= n = i
                | otherwise  = findK (i+1)
        go _ [] = blank
        go i (b:bs) =
          translated (fromIntegral (i `mod` k))
                     (-fromIntegral (i `div` k))
                     (pictureOfBool b)
          & go (i+1) bs

        pictureOfBool True =  colored green (solidCircle 0.4)
        pictureOfBool False = colored red   (solidCircle 0.4)
        
isMazeClosedAndSane :: Maze -> Bool
isMazeClosedAndSane maze = (isClosed maze) && (isSane maze)

areMazesOk :: [Maze] -> [Bool]
areMazesOk mazeList = mapList isMazeClosedAndSane mazeList
        
-- UWAGA: MICROCOSMOS01 NIE JEST 
mazesSanity :: [Bool]
mazesSanity = areMazesOk mazes
    
badMazesSanity :: [Bool]
badMazesSanity = areMazesOk badMazes

printList list = drawingOf (lettering (Text.pack (show list)))

-- Założyłem, że na ekranie startowym chcemy jedynie wyświetlać wyniki dla dobrych plansz.
etap4 :: Picture
etap4 = pictureOfBools mazesSanity

main :: IO ()
main = runActivity (Activity (initialState sampleMaze) handleEvent draw)
