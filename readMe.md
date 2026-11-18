# Maze Explorer

A C program that loads or generates mazes and animates a robot in DrawApp.
The fixed maze and animated robot show visit colours and movement statistics.

The default search uses right-hand wall following. At each square the robot
checks for the exit, then tries right, straight ahead, left and finally back.
It turns to inspect passages using `canMoveForward()` and moves with `forward()`;
`left()` and `right()` change its heading, while `atMarker()` detects the finish.
Dead ends make it turn back; visit colours are display data, not a solver map.
It may revisit cells and does not find the shortest route. Generation uses
recursive backtracking to carve passages, with optional extra openings for loops.
A separate breadth-first search checks reachability before traversal. Optional
IDDFS repeats searches with increasing depth limits and physically backtracks.

Compile once from the source directory with the unmodified course `graphics.c`,
`graphics.h` and `drawapp-5.0.jar` beside the sources. Use Clang (or GCC) and
a Java runtime compatible with DrawApp:

```sh
clang -std=c17 -Wall -Wextra -Wpedantic -DMAZE_DRAWAPP \
  maze_main.c maze_config.c maze_grid.c maze_generator.c \
  maze_robot.c maze_solver.c maze_render.c graphics.c -o maze
```

Terminal format: `./maze [OPTIONS] | java -jar drawapp-5.0.jar`.
Replace `[OPTIONS]` with flags below, or omit it for the default maze; do not type
the brackets. Put all maze options before `|`. Run one command at a time and
close each drawing window before starting another. Copyable examples:

```sh
./maze | java -jar drawapp-5.0.jar
./maze --preset large | java -jar drawapp-5.0.jar
./maze --preset loops --delay 20 | java -jar drawapp-5.0.jar
./maze -f mazes/branching_maze.txt | java -jar drawapp-5.0.jar
./maze --generate 51 51 --seed 42 --loops 25 --cell-size 12 --delay 5 \
  | java -jar drawapp-5.0.jar
```

These run the default, large, looped, file-loaded and custom 51-by-51 mazes.
For terminal-only output or help, omit the DrawApp pipe:

```sh
./maze --generate 35 35 --seed 42 --headless
./maze --help
```

`--generate WIDTH HEIGHT` requires odd sizes from 5 to 101; `--seed N` repeats
a layout, `--loops 0-100` controls extra passage creation, `--delay N` sets ms
per action, and `--cell-size N` sets zoom (0 automatic). IDDFS can be expensive;
try small mazes with `./maze --solver iddfs --headless --budget 10000`.
A budget stop does not mean no route exists. Maze files use `#` walls, spaces/`.`
paths and one boundary `S` entrance and `E` exit; the robot starts facing inward.
Diagnostics use stderr; buffered DrawApp playback can outlast the C process.

Local shortcut: from `CS Work`, use `./run.sh [OPTIONS]`; from `c-coursework`,
use `../run.sh [OPTIONS]`, with the same options and no pipe. For example:
`../run.sh --preset loops --delay 20`. It builds and opens DrawApp automatically.
