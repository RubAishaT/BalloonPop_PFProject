# Balloon Pop : Connect & Match

A colourful balloon-popping puzzle game written in **C** for Windows.
Programming Fundamentals mini project.

- One source file: `balloon_pop.c` (about 1,400 lines, heavily commented)
- Uses only the Windows API that comes with the compiler. No extra libraries to install.
- 5 levels, score, combo bonus, bomb power-up, high scores, pause/restart, help screen.

![Main menu](docs/screenshots/01_menu.png)
![Playing](docs/screenshots/04_play_start.png)

---

## 1. How to run

**Quickest way:** double-click `BalloonPop.exe`.

**Build it yourself with Code::Blocks (MinGW):**

1. Open `BalloonPop.cbp` in Code::Blocks.
2. Press **F9** (Build and Run).

**Build it from the command line:** double-click `build.bat`, or run:

```
gcc balloon_pop.c -o BalloonPop.exe -std=gnu99 -mwindows -lgdi32 -luser32 -lwinmm
```

**VS Code:** install MinGW, open this folder, open a terminal and run the command above.

> The game saves your best scores in a small text file called `highscores.txt`
> next to the exe. It is created automatically.

---

## 2. How to play

1. Click **Start Game** on the main menu.
2. Click any group of **2 or more touching balloons of the same colour** (up and down, left and right; diagonals do not count).
3. The group pops, balloons above fall down, and you score points.
4. Reach the **target score** before you run out of **moves** or **time**.

| What | Rule |
|------|------|
| Score for a group | size x size x 5 (bigger groups are worth much more). Levels with more colours are harder, so balloons there are worth a bit more (x1.5 with 4 colours, x2 with 5) |
| Combo | Every good pop in a row raises the multiplier: x1.25, x1.50 ... up to x2.00. Clicking a single balloon resets it |
| Bomb | Pop a group of **6 or more** and a black bomb balloon appears. Click it to blast the 3x3 area around it |
| Wrong click | Clicking a single balloon costs nothing, but it breaks your combo |
| Cleared board | Bonus points and a fresh board |
| Level complete | Bonus for unused moves and time left, plus 1 to 3 stars |
| Game over | Out of moves, out of time, or no matching balloons left |

| Level | Colours | Target (points in that level) | Moves | Time |
|:-----:|:-------:|:-----------------------------:|:-----:|:----:|
| 1 | 3 | 3400 | 20 | 90 s |
| 2 | 3 | 3700 | 18 | 85 s |
| 3 | 4 | 3900 | 18 | 80 s |
| 4 | 4 | 4050 | 15 | 75 s |
| 5 | 5 | 4250 | 15 | 70 s |

The target counts only the points you earn **in that level**. The side panel shows it as `earned / target`.
A level usually takes about 10 to 14 pops, so you have to plan your moves.

Each colour also has its own **symbol** (star, diamond, circle, triangle, square),
so the game is playable even for colour-blind players.

### Keys

| Key | Action |
|-----|--------|
| Mouse click | Pop balloons / press buttons |
| **P** or **Esc** | Pause / resume |
| **R** | Restart the level |
| **M** | Sound on / off |
| **Enter** | Start / next level / play again |

---

## 3. Features checklist (from the project proposal)

| Proposal feature | Where it is |
|------------------|-------------|
| Main menu: Start, Instructions, Exit | `DrawMenu`, `SetupButtons` |
| Grid of coloured / symbol balloons | `board[ROWS][COLS]`, `DrawBalloon` |
| Click connected same-colour groups | `FindGroup`, `SpreadGroup` (recursion) |
| Score system | `PopGroup`, `score` variable |
| Levels with increasing difficulty | `levelColours`, `levelTarget`, `levelMoves`, `levelSeconds`, `LevelScore` |
| Limited moves **and** countdown timer | `movesLeft`, `CountDownTime` |
| Combo bonus | `ComboPercent` |
| Special bomb power-up | `ExplodeBomb` |
| High score display (saved to file) | `LoadHighScores`, `SaveHighScores`, `AddHighScore` |
| Pause / restart / exit | `DrawPaused`, `HandleKey` |
| Help / instructions screen | `DrawHelp` |
| Sound and visual feedback | `MakeSound`, `PlaySoundEffect` (short pop sounds made by the program itself), `SpawnEffects` (popping animation) |
| Game over / victory / level complete screens | `DrawGameOver`, `DrawWin`, `DrawLevelDone` |
| Play again | `BTN_AGAIN` button |
| Input validation | `PlayerClick` ignores clicks outside the grid, on empty cells and on single balloons |
| Modular functions | every job has its own small function |
| Random board generation | `FillBoard` (uses `rand()`) |

---

## 4. Code walkthrough

Read `balloon_pop.c` from top to bottom. It is split into 7 parts:

| Part | What it contains |
|------|------------------|
| 1 | Constants (`#define`), enums and global variables |
| 2 | **Game logic.** No drawing here, only the rules |
| 3 | High scores: reading and writing `highscores.txt` |
| 4 | Drawing helpers: text, shapes, balloons, backgrounds |
| 5 | The screens: menu, help, playing, paused, level done, game over, win |
| 6 | Mouse and keyboard handling, game flow |
| 7 | The Windows part: opens the window and runs the timer |

**The most important function** is `SpreadGroup`. It uses **recursion** to find every
balloon connected to the one you clicked: it checks the four neighbours, and each
matching neighbour does the same.

**One game turn:**

```
click  ->  FindGroup  ->  size >= 2 ?  ->  PopGroup  ->  DropBalloons  ->  ShiftColumns
                              |                                              |
                          no: reset combo                       CheckLevelEnd (win / lose?)
```

### Programming concepts used

- variables, constants (`#define`) and `enum`
- 2D array (`board[ROWS][COLS]`)
- `if / else`, `switch`, `for` and `while`
- functions with parameters and return values
- **recursion** (flood fill in `SpreadGroup`)
- one `struct` (`Button`)
- file handling (`fopen`, `fscanf`, `fprintf`, `fclose`)
- **bubble sort** to keep the top 5 high scores
- random numbers (`rand`, `srand`)
- event-driven programming (mouse, keyboard and timer messages)

---

## 5. Testing

The game rules (everything in Parts 1 to 3) were tested automatically:
**182 checks pass**, including a stress test with 20,000 random clicks and key
presses, run with memory checkers (AddressSanitizer and UBSanitizer).
Two bots (one clicking at random, one that thinks a move ahead) played every level 200 times to check that the difficulty rises steadily: level 1 is easy and level 5 is hard to beat. Run `./run_tests.sh sim` on Linux to see the table.
The `tests/` and `tools/` folders are only for this testing. You can ignore them when submitting or reading the game.

Every screen was rendered to a picture and checked by eye (see `docs/screenshots/`).

**Honest note:** `BalloonPop.exe` was built with a cross-compiler on Linux (`x86_64-windows-gnu`)
and has not been run on a real Windows PC by the author. If it does not start for any reason,
rebuild it with `build.bat` or Code::Blocks. The source code is the same.

---

## 6. Ideas to extend the project

- Add a "hint" button that highlights the biggest group.
- Load real sound files (.wav) instead of the sounds the program generates.
- Save the player's name with each high score.
- Add more levels by extending the four level tables.

## Screenshots

| Help | Bomb | Level complete |
|------|------|----------------|
| ![](docs/screenshots/02_help.png) | ![](docs/screenshots/08_bomb_made.png) | ![](docs/screenshots/13_level_done.png) |

| Game over | You win |
|-----------|---------|
| ![](docs/screenshots/15_game_over.png) | ![](docs/screenshots/16_win.png) |
