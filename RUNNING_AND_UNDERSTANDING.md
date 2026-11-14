# Running and understanding the maze project

The graphical version has now been checked in the genuine DrawApp 5.0 viewer.
For a view of the current design, see [the real-window screenshot](docs/drawapp-preview.png).
It includes a 35x35 maze, exploration trace and statistics dashboard.

This guide explains the current learning/reference program. The normal launcher
opens an animated DrawApp window. Text preview and headless search are explicit
alternative modes for inspecting data and running quick checks.

## 1. Open the correct folder

Open the VS Code terminal and enter:

```sh
cd "/Users/diartolluri/Main/Programming/CS Work/c-coursework"
```

All commands below assume you are in that directory. You can check with `pwd`.

The launcher needs clang, make, the supplied DrawApp files, and Java 21 or later.
The course graphics files are present in this project. Java can come from
`JAVA_HOME`, the local `.runtime` installation, or the system Java command. See
`DRAWAPP_SETUP.md` if runtime setup is needed. Python 3 is used only by the
development tests and packaging script. Text modes do not need Java.

## 2. Your first run: watch the robot

```sh
./run.sh
```

This builds the graphical C executable and opens the real DrawApp 5.0 viewer.
You should see a maze and an arrow-shaped robot moving and turning as it
searches. The maze is the fixed 9-by-7 example unless you request a file or
generation. The default solver follows the right wall: try right, then forward,
then left, and finally turn back if necessary. It finds the boundary exit without
using a map. It can explore dead ends and pass beside the exit before entering
it: the only exit sensor is `atMarker()` on the robot's current square.

Use **Pause / Play** in the viewer to pause/resume playback. **Next** and
**Previous** step through drawing commands, which may be smaller than one robot
move. **Reset** rewinds retained drawing history; it does not generate a new
maze or restart the C algorithm. **Close Window** closes the viewer. Ctrl-C in
the launching terminal is another way to stop it.

For ready-to-run demonstrations:

```sh
./run.sh --preset small
./run.sh --preset large
./run.sh --preset loops
```

You can explicitly request text output instead:

```sh
./run.sh --preview
./run.sh --headless
```

`--preview` prints the initial grid without searching. `--headless` computes the
search and prints its final grid; it does not animate the terminal. Build/status
messages go to stderr. A "nothing to be done" build message means the executable
is already up to date.

The initial default maze looks like this in text mode:

```text
#########
S   #   #
### # # #
#   # # #
# ### # #
#       E
#########
```

| Symbol | Meaning |
| --- | --- |
| `#` | Wall |
| Space or `.` | Empty passage |
| `S` | Starting entrance |
| `E` | Exit marker |
| `^`, `>`, `v`, `<` | Robot facing north, east, south, or west |

The preview shows `S` and `E`. After a search, the robot arrow is drawn on top of
the cell it occupies. On success it therefore covers `E`; the exit has not been
removed from the underlying maze.

## 3. Commands worth trying

| Preset | Settings |
| --- | --- |
| `small` | Fixed 9x7 example, wall-following, 50 ms/action. |
| `large` | Generated 35x35, seed 42, no added loops, 20 ms/action. |
| `loops` | Generated 15x11, seed 42, loops 25%, 35 ms/action. |

Use `--preset NAME`. Explicit arguments override preset defaults, even when they
appear before `--preset`. For example, `./run.sh --preset loops --delay 10`.
Do not combine a generated preset with `--file`; choose one maze source.

### Does the coursework require maze generation?

The question sheet's Basic Stage 1 asks for a designed maze and suggests reading
its data from a file. Stage 4 then asks for an algorithm that generates mazes
with one entrance and one exit. Stage 5 asks for the robot search to work on any
maze the program generates. Generation is therefore part of the later stages;
the earlier display and movement stages do not require it.

This program supports both. `./run.sh` and `--preset small` use the designed
example; `--preset large`, `--preset loops`, and `--generate WIDTH HEIGHT` actually
run the generator. The maze is constructed before the robot begins exploring;
the robot does not receive the generator's data. Generation is not animated.

### Generate a new maze and watch it being solved

```sh
./run.sh --generate 15 11 --seed 42 --solver wall --delay 50
```

This makes a maze 15 columns wide and 11 rows high. A seed is a starting value
for the generator's random-number sequence: the same dimensions, seed, and loop
setting reproduce the same maze in this program.
Change the seed, for example to `--seed 99`, to try another layout. Different
seeds can occasionally give the same result; a seed is not a unique maze ID.

Generated dimensions must be odd numbers from 5 to 101. Without `--generate` or
`--file`, the program uses the fixed 9-by-7 maze shown above.

### Try a maze with cycles

```sh
./run.sh -f mazes/looped.txt --delay 10
```

This uses the default wall follower. Add `--solver iddfs` for the more expensive
iterative-deepening comparison, or `--headless` for results without playback.

Or generate a small example with all eligible cycle-forming walls opened:

```sh
./run.sh --generate 9 7 --seed 7 --loops 100 --headless
```

`--loops 25` gives each eligible internal wall a 25% chance of being opened
after the initial maze is carved. It does not mean that exactly 25% of the maze
will consist of loops. Larger cyclic mazes can make the optional IDDFS search expensive.

### Compare the two solvers without playback delays

```sh
./run.sh --generate 15 11 --seed 42 --solver wall --headless
./run.sh --generate 15 11 --seed 42 --solver iddfs --headless
```

Wall-following is much faster here. The coursework fixes a boundary entrance,
inward-facing start and reachable boundary exit, so following the outside wall
works even with passage loops. This guarantee does not extend to arbitrary
interior starts or interior goals. IDDFS is an optional comparison and can repeat
many journeys. Neither complete animated journey is guaranteed to be shortest.

### Watch the visit colours change

```sh
./run.sh --preset small --solver iddfs --delay 5
```

This small comparison repeats paths often, so the squares progress from mint
through lavender to rose. It takes about 6.5 seconds of requested playback plus
drawing overhead. Use `--delay 20` for a slower view. Wall-following remains the
quicker default; IDDFS is useful here for demonstrating the colour bands.

### Save a maze and load it again

```sh
./run.sh --generate 11 9 --seed 42 --save my-maze.txt --preview
./run.sh -f my-maze.txt --headless
```

The first command writes the initial maze into `my-maze.txt` and displays it.
The second reads the same file and solves it. `--save` replaces the named output
file if it already exists.

### Show every option

```sh
./run.sh --help
```

| Option | What it changes |
| --- | --- |
| `--preset small`, `large`, `loops` | Selects the demonstration settings listed above; explicit options override them. |
| `--generate W H` | Generates an odd-width/odd-height maze; both dimensions must be 5..101. |
| `--seed N` | Sets the reproducible generator seed (default 1 outside presets); requires generation. |
| `--loops N` | Sets each eligible wall's opening probability, 0..100% (default 0); requires generation. |
| `-f PATH` or `--file PATH` | Loads a maze instead of generating one. |
| `--solver wall` or `--solver iddfs` | Chooses right-wall following (default) or iterative deepening. |
| `--delay N` | Sets 0..1000 milliseconds per displayed action (default 50 outside presets). |
| `--budget N` | Stops after at most N movement/turn actions (default 2,000,000); 0 removes the guard. |
| `--save PATH` | Saves the initial maze, replacing an existing file at PATH. |
| `--preview` | Shows the initial text maze and exits without searching. |
| `--headless` | Solves and prints the final text maze without opening DrawApp. |
| `--graphics` | Explicitly requests DrawApp animation, which the launcher uses by default. |
| `--commands` | Emits drawing commands for redirection into a replay file; does not open DrawApp. |
| `--help` | Lists supported options and exits. |

Choose one output mode per run. Numeric option values are whole nonnegative
numbers. `--delay` changes animation timing; it does not change the route chosen.

You can also build separately and run the executable directly:

```sh
make
./build/maze --headless
```

After editing C code, run `make` or use `run.sh` again so that your changes are
compiled before the executable starts.

## 4. Understanding the statistics

### The on-screen cell key

The key stays below the maze throughout playback:

| Colour or shape | Meaning |
| --- | --- |
| Dark navy | Wall |
| White | Passage the robot has not entered |
| Mint green | Passage entered once |
| Lavender | Passage entered two or three times |
| Rose pink | Passage entered four or more times |
| Blue | Entrance; retains its identity as visits accumulate |
| Gold | Exit marker; retains its identity when reached |
| Teal triangle | Robot and its current direction |
| Teal outline ahead | Front sensor reports a clear square |
| Amber outline ahead | Front sensor reports a blocked square |

The starting entrance counts as one arrival. Only moving into a square increases
its visit count; turning and redrawing do not. On larger tiles a small `1`, `2`,
`3`, or `4+` also shows the count; on compact mazes a dot marks visited cells.
Counts are stored only up to four, because four and higher share one band.
The robot covers the tile beneath it, so a fill is easiest to see after it leaves.
See [the repeated-visit screenshot](docs/drawapp-visit-key.png) for every visit
band and numeric labels in the real viewer.

These colours describe physical exploration. They do not identify the shortest
route, prove a dead end, or mean that a branch has been completely searched.
White squares can remain when the exit is found. This display state belongs to
the renderer and is never available to the solver.

### Counters and timing

Solving the fixed default maze currently prints a line like:

```text
solved: 16 moves, 32 turns, 48 actions; 0 iterations, depth 0.
```

- **Moves:** successful one-square forward movements, including backtracking.
- **Turns:** individual 90-degree rotations.
- **Actions:** the search's movement/turn calls. These solvers check the sensor
  before moving, so successful runs have actions equal to moves plus turns.
- **Iterations:** how many depth-limited searches were attempted by IDDFS.
- **Depth:** the forward-move limit of the successful search pass. For IDDFS
  this equals the shortest entrance-to-exit distance in squares.

Wall-following has no depth-limit passes, so the text summary's iteration/depth
fields are 0; the graphical panel shows its priority rule instead. With
`--solver iddfs`, this same maze takes 262 moves and 1,044 turns to find depth 16.
That includes repeated exploration and backtracking, not just the final route.

Additional graphical feedback:

- **Front sensor:** clear or blocked for the square immediately ahead. Its outline
  uses the same colours. The renderer calculates this presentation from the world;
  it supplies no observations or map to the solver.
- **Decision:** explains the action being animated, such as inspecting right,
  trying forward after a blocked passage, or turning back at a dead end.
- **Revisits:** forward arrivals at cells already reached; turning does not count.
- **Coverage:** unique cells physically reached out of all open cells, not a
  percentage of progress toward the exit.
- **Final new-cell rate:** unique entered cells divided by total arrivals,
  including the starting cell. It measures repeated exploration, not optimality.
- **Final CPU time:** C search and drawing-command generation time. It excludes
  the Java viewer's playback and pauses; playback time is shown separately.

Static dashboard labels are drawn once. Changing values update at about 25 Hz
or less (every 32 actions for zero-delay records); final values always update.
Decision feedback and the robot animation still follow each action.

In text mode, grid output goes to **stdout**; statistics and errors go to
**stderr**. In graphics mode, stdout is reserved for drawing instructions sent
to DrawApp. The renderer's visual statistics or visit presentation are display
state only; the solver receives no map from them.

A terminal "solved" line can appear before the robot visibly reaches the exit:
the C program has finished computing, but DrawApp may still be playing buffered
actions. Increasing `--delay` changes playback time, not the search algorithm
or headless computation speed. To save text output and statistics separately
after building:

```sh
./build/maze --headless > final-maze.txt 2> search-log.txt
```

## 5. What happens inside the program

The overall sequence is:

```text
Read options
    -> create, load, or generate a maze
    -> validate the maze
    -> optionally save it
    -> preview only, OR create a robot and run a search
    -> display the result and statistics
    -> release allocated memory
```

| File | Responsibility |
| --- | --- |
| `main.c` | Coordinates that sequence and handles setup/cleanup. |
| `config.h` / `config.c` | Defines the options and parses command-line arguments. |
| `maze.h` / `maze.c` | Stores tiles, checks coordinates, loads/saves files, and validates maze rules. |
| `generator.h` / `generator.c` | Creates reproducible mazes and optionally opens walls to introduce loops. |
| `robot.h` | Exposes only the five movement/sensor operations to the solver. |
| `robot_internal.h` / `robot.c` | Implements the robot's private state, world setup, display view, and counters. |
| `solver.h` / `solver.c` | Implements iterative-deepening search and wall-following. |
| `render.h` / `render.c` | Prints the text grid or emits graphical drawing calls. |
| `Makefile` | Builds the executable and provides test/graphics/package commands. |
| `run.sh` | Builds, finds Java, and launches the graphical pipeline by default; selects explicit text/command modes. |
| `mazes/` | Contains six example input files. |
| `tests/` | Contains behaviour and interface checks used during development. |

Headers (`.h`) declare types/functions shared with other files. Source files
(`.c`) implement their behaviour. The compiler builds the source files and links
them into one executable.

### The maze knows the world; the solver does not

The maze stores its tiles in a dynamically allocated block of memory. Its logical
coordinates use x for columns and y for rows, with `(0, 0)` at the top left.
The robot implementation can check those tiles for collision detection.

The solver receives an opaque `Robot *`: it can refer to a robot, but its public
header does not reveal the position, direction, or maze fields. Its only robot
operations are:

```c
forward(robot);
left(robot);
right(robot);
atMarker(robot);
canMoveForward(robot);
```

`forward`, `left`, and `right` change its state. `atMarker` and
`canMoveForward` answer questions with an integer truth value.

The maze validator uses a separate full-grid traversal to reject unreachable
input. That traversal does **not** give the solver a route, visited matrix, or
other map information.

### Why wall-following works here

The right-first rule traces the boundary of a face in the planar passage graph.
The boundary entrance selects the exterior face. A reachable boundary exit lies
on that same face, so the traversal reaches it, including when interior walls
form islands. This argument depends on the exact static, boundary-to-boundary
maze rules, which loading and generation enforce. The permanent test suite also
exhausts all 18,676 reachable 5x5 boundary configurations.

### How the optional iterative-deepening search works

Select **iterative-deepening depth-first search**, abbreviated IDDFS, with
`--solver iddfs`. It searches paths of at most one forward move, then two, then three, and
so on, until it reaches the marker.

During a pass it tries available relative directions, moves physically, and
backtracks physically when a branch fails. A failed branch restores the position
and heading it started with. It checks for the marker immediately after moving.

A linked stack of small heap-allocated frames stores its active action history:
which direction to try next, how much depth remains, and how to undo its turn.
This is not a map of visited cells. The root tries all four directions;
descendants skip immediately reversing the last move, since such an out-and-back
step is unnecessary in a shortest path.

Each pass has a finite depth, so it cannot keep exploring a cycle forever.
Increasing the depth eventually includes a route to any reachable exit,
provided memory and the configured action budget are sufficient. Repeated
exploration can nevertheless take exponential time in cyclic mazes.

### How generation works

The generator starts with walls and recursively carves passages between logical
cells. Each newly reached logical cell is marked before continuing, so the
initial passages form a connected maze without cycles. It then optionally opens
eligible internal walls, adds the entrance/exit, and validates the result.

This uses recursion, unsigned direction masks, a reproducible random-number
sequence, and dynamically allocated maze storage. Search frames and maze storage
have explicit owners and are freed when finished.

## 6. Making your own maze file

Copy `mazes/simple.txt`, edit the copy in a plain-text editor, and run:

```sh
./run.sh -f my-maze.txt --headless
```

The loader requires:

- Between 5 and 101 rows and columns; every row has the same width.
- Only `#`, spaces, `.`, `S`, and `E`.
- Exactly one `S` and one `E`, each on a non-corner boundary cell.
- Walls on every other boundary cell.
- At least one horizontal/vertical passage from `S` to `E`.

Loaded dimensions can be even; only generation requires odd dimensions. The
robot's initial facing is inferred from the entrance edge. LF and CRLF line
endings are accepted, as is a final row without a newline. Blank rows and unknown
characters are rejected.

## 7. Graphics commands, timing, and replay

The course's matching, unmodified `graphics.h`, `graphics.c`, and
`drawapp-5.0.jar` are now in this folder. `run.sh` builds the graphical executable
and connects its command output to Java. The pipe carries drawing instructions;
the Java program displays them.

The maze is drawn once on the background layer. A callback tells the renderer
when the robot changes; it updates the foreground robot and display information.
Only the renderer handles pixels and visual presentation. The solver chooses
its actions through the five robot functions.

`--delay 100` requests 100 milliseconds between displayed actions. Reduce the
delay for IDDFS, which can issue many more actions than wall-following. Turns
also count as actions. With `--solver iddfs`, the fixed maze's 1,306 search
actions take roughly 65 seconds at 50 ms per action before drawing overhead;
the default wall follower uses 48 actions, about 2.4 seconds. Pauses make
playback longer. Headless computation does not wait for this animation delay.

To emit graphics instructions without opening a window:

```sh
./run.sh --commands --delay 25 > build/replay.draw
```

Then, with Java 21 or later on PATH:

```sh
java -jar drawapp-5.0.jar < build/replay.draw
```

If using the local runtime rather than system Java, substitute
`.runtime/java/Contents/Home/bin/java` for `java`. Replay uses the saved drawing
commands and does not run the C search again. Avoid feeding text-preview output
or diagnostic messages into the viewer.

The real viewer retains a limited command history. A **History full** warning
means older instructions are discarded from that history; drawing continues.
Previous and Reset can then access only the retained portion. These controls
operate on instructions, so they do not necessarily align with whole robot moves.

See `DRAWAPP_SETUP.md` for manual compile/run commands. `VALIDATION.md` records
which actual-library and live-window checks have been completed.

## 8. Common problems

| Message or symptom | Explanation and next action |
| --- | --- |
| `No such file or directory` for `./run.sh` | Check `pwd`; enter the `c-coursework` folder first. |
| Input file cannot be opened | Use a path relative to the project folder, or an absolute path. |
| Generated dimensions rejected | Use two odd values between 5 and 101, such as `15 11`. |
| `--seed` or `--loops` rejected | Supply them with `--generate`, not a file or the default maze. |
| `action budget exhausted` | The search reached its action guard; this does not mean the exit is unreachable. |
| Large cyclic maze takes a long time | Try a smaller cyclic maze, fewer opened walls, or a larger finite budget. Use the default wall follower for supported boundary-to-boundary mazes. |
| No graphical window | Remove `--headless`, `--preview`, or `--commands` if you intended animation. Check the Java/setup message. A desktop display is required. |
| Java cannot be found or launched | Set `JAVA_HOME` to a Java 21+ installation or use the project-local runtime; `--headless` and `--preview` still work. |
| Terminal says solved while the robot is still moving | DrawApp is playing buffered commands. Let playback finish or reduce `--delay` for the next run. |
| `History full` message | Playback continues, but Previous/Reset are limited to the retained history. |

The default budget is 2,000,000 movement/turn actions. For example:

```sh
./run.sh -f mazes/looped.txt --headless --budget 5000000
```

`--budget 0` removes the guard, so an expensive search can run for a very long
time. Ctrl-C stops the running process. A search stopped by a budget may leave
the robot partway through backtracking; restoring the initial pose is guaranteed
for completed failed branches, not an interrupted search.

Exit status 0 means success, 1 means an input/setup error, and 2 means the search
did not finish successfully. In a terminal, `echo $?` shows the status of the
most recently completed command.

`run.sh` changes into its own project directory before running. If you launch it
from the parent workspace as `./c-coursework/run.sh`, file arguments are still
relative to `c-coursework`. A directly launched `build/maze` instead inherits the
directory you launch it from. Starting with the `cd` command in section 1 avoids
this difference.

## 9. Checks and further reading

```sh
make test
make sanitize
```

The first command runs robot/search/generator/file/CLI and launcher-routing
checks, and verifies that the solver depends only on the five permitted robot
operations. The second runs
checks with memory/undefined-behaviour diagnostics. Inspect the actual viewer too:
check the robot heading, clean foreground updates, readable display, boundary
clipping, and successful arrival. `VALIDATION.md` is the record of performed checks.

Use `make package` to create `build/COMP0002CW1.zip` from the explicit source,
fixture, and short-readMe allowlist. This long learning guide is not packaged.
Packaging also extracts the ZIP into a temporary directory, runs its README
compilation command against the supplied graphics files, and checks the resulting
program. The short README uses direct commands so it works without `run.sh`.

- `README.md`: short command reference.
- `DESIGN.md`: design rationale, algorithm references, and lecture mapping.
- `IMPLEMENTATION_GUIDE.md`: the original coursework/lecture study notes.
- `VALIDATION.md`: what has actually been tested and what remains unverified.
- `TEST_PLAN.md`: the broader acceptance checklist.
- `REQUIREMENTS_AUDIT.md`: every coursework requirement mapped to code/evidence.

For reading the implementation, start with `main.c`, then `config.c`, `maze.c`,
`robot.c`, `solver.c`, and `render.c`. Return to `generator.c` after you understand
how a fixed maze and its robot work together.
