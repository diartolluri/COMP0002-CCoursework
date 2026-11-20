# Maze Explorer - C project

Maze Explorer is a C17 program that guides a robot from the entrance to the exit
of a rectangular maze. You can try the built-in maze, load one from a text file,
or generate one from a seed. DrawApp 5.0 animates the robot's decisions, and
terminal mode prints the maze and a summary of the run.

## How the robot searches

The default solver follows the wall on its right. At each square it checks for
the exit, then looks for an open passage to the right, straight ahead, to the
left, or behind it. It turns to face each candidate passage before checking
`canMoveForward()`. The solver receives its information through that sensor and
`atMarker()`, and moves using `forward()`, `left()`, and `right()`. It can revisit
squares and take a longer route than necessary.

The optional iterative deepening depth-first search (`--solver iddfs`) explores
up to a depth limit, backtracks through the maze, and tries again with a larger
limit. Separately, the program checks that the entrance and exit are connected
using breadth-first search before the robot starts. Algorithm references are
included in the source comments.

## Build and run

From the repository root, use Make and Clang to try the terminal version without
the DrawApp files or Java.

```sh
make
./build/maze --headless
```

Place the course's **unmodified** `graphics.h`, `graphics.c` and
`drawapp-5.0.jar` beside the sources to enable animation. With Clang and Java 21+
on your `PATH`, run `./run.sh --preset large` to build the graphical version and
open DrawApp. You can also compile and launch it manually with Clang or GCC.

```sh
clang -std=c17 -Wall -Wextra -Wpedantic -DMAZE_DRAWAPP \
  main.c config.c maze.c generator.c robot.c solver.c render.c graphics.c -o maze
./maze --preset large | java -jar drawapp-5.0.jar
```

## Try different mazes

```sh
./maze | java -jar drawapp-5.0.jar
./maze --preset loops | java -jar drawapp-5.0.jar
./maze --solver iddfs --delay 5 | java -jar drawapp-5.0.jar
./maze -f mazes/branching.txt --delay 50 | java -jar drawapp-5.0.jar
./maze --generate 21 15 --seed 99 --save example.txt --preview
./maze -f example.txt --headless
./maze --help
```

The default maze is a designed 9×7 grid. The `large` and `loops` presets generate
seeded mazes. Generation carves passages by backtracking and can add extra
openings to create loops. Reuse the same seed and generation settings to
reproduce a layout. Explicit options override presets regardless of argument
order.

Maze files use `#` for walls and spaces or `.` for passages. Place one `S`
entrance and one `E` exit on non-corner boundary squares and connect them with
a route. Width and height can range from 5 to 101. Generated mazes require odd
dimensions. `--preview` prints the initial grid and exits, and `--save` writes
that grid to the named file, replacing it if it already exists.

## Read the animation and results

The on-screen key labels white unvisited, mint one arrival, lavender 2-3 and rose
4+ arrivals. Turning in place leaves the count unchanged. The entrance stays
blue and the exit stays gold. The dashboard shows actions, the share of passages
visited, and whether the way ahead is clear. Use Pause/Play to control playback
or Ctrl-C to interrupt a run.

Graphics commands go to standard output and diagnostics go to standard error.
`--delay` sets the time per displayed action from 0 to 1,000 milliseconds.
DrawApp may finish playback after the C process exits.

IDDFS can take many actions in mazes with loops. By default, the robot stops
after 2,000,000 moves and turns combined. Use `--budget 0` to remove that limit.
The exit code is `0` for success, `1` for a setup or input error, and `2` if the
search ends before reaching the exit.
