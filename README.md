# Maze Explorer - C project

DrawApp 5.0 animates a robot solving designed or generated rectangular mazes.
The default right-wall follower uses only `forward`, `left`, `right`, `atMarker`
and `canMoveForward`; it receives no map. Optional IDDFS demonstrates repeated
exploration. This learning/model solution includes algorithm references in the
source comments.

Place the course's **unmodified** `graphics.h`, `graphics.c` and
`drawapp-5.0.jar` beside these sources. They are intentionally excluded from this
ZIP. With clang (or substitute gcc) and Java 21+ on PATH, compile and run here:

```sh
clang -std=c17 -Wall -Wextra -Wpedantic -DMAZE_DRAWAPP \
  main.c config.c maze.c generator.c robot.c solver.c render.c graphics.c -o maze
./maze --preset large | java -jar drawapp-5.0.jar
```

Other demonstrations:

```sh
./maze | java -jar drawapp-5.0.jar
./maze --preset loops | java -jar drawapp-5.0.jar
./maze --solver iddfs --delay 5 | java -jar drawapp-5.0.jar
./maze -f mazes/branching.txt --delay 50 | java -jar drawapp-5.0.jar
./maze --generate 21 15 --seed 99 --save example.txt --preview
./maze -f example.txt --headless
./maze --help
```

Default: designed 9x7 maze. Presets `large`/`loops` generate seeded mazes; explicit
options override presets in either order. Identical seed/settings reproduce a
layout. Input uses `#` walls, spaces/`.` passages and one non-corner boundary
`S` and `E`, connected by a route. Dimensions: 5..101; generation requires odd
dimensions. `--save` replaces the named file.

The on-screen key labels white unvisited, mint one arrival, lavender 2-3 and rose
4+; turns do not count. Entrance blue and exit gold retain their identity. The
dashboard shows actions, coverage and front-sensor feedback. Coverage is not
progress toward the exit. Pause/Play controls playback; Ctrl-C interrupts a run.

Graphics commands use stdout; diagnostics use stderr. `--preview` shows the
initial text grid; `--headless` solves without Java. `--delay` sets 0..1000 ms
per displayed action. DrawApp may finish playback after the C process finishes.
IDDFS can be expensive on cycles. The default 2,000,000-action budget reports a
resource stop; `--budget 0` removes it. Exit codes: 0 success, 1 setup/input error,
2 unsuccessful/resource-limited search.
