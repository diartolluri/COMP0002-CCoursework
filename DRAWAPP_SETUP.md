# DrawApp 5.0 setup and playback

The graphical build uses the course's unmodified `graphics.h`, `graphics.c`, and
`drawapp-5.0.jar`. Copies of the supplied files are in the project directory.
Their originals were provided in:

```text
/Users/diartolluri/University Work/COMP0002 - Principles of Programming/drawapp
```

The three project copies are byte-identical to the supplied files and remain
unmodified. No replacement graphics library is required.
The JAR's class-file version is 65, requiring Java 21 or later.

## Start the graphical program

From `c-coursework`:

```sh
./run.sh
```

The launcher builds the DrawApp-enabled C executable and connects its command
output to the Java viewer. It searches `JAVA_HOME`, the local project runtime
at `.runtime/java/Contents/Home/bin/java`, and the system Java command, in that
order. A project-local Temurin 21 runtime is configured on this computer.
If Java is unavailable, install a compatible runtime or point `JAVA_HOME` at one.
The project-local runtime is development tooling, not part of your C program.

For a generated maze with the simpler, quicker solver:

```sh
./run.sh --generate 15 11 --seed 42 --solver wall --delay 50
```

For a cyclic example with the default wall follower:

```sh
./run.sh -f mazes/looped.txt --delay 10
```

`--headless` explicitly selects terminal computation, and `--preview` displays
only the initial text grid. These modes do not need Java. Normal animation
needs a desktop display; a server without a display can still run the tests
and generate replay commands.

## Manual compilation and replay

With the graphics files beside your source, build with standard clang or gcc:

```sh
clang -std=c17 -Wall -Wextra -Wpedantic -DMAZE_DRAWAPP main.c config.c maze.c generator.c robot.c solver.c render.c graphics.c -o maze
./maze --delay 25 | java -jar drawapp-5.0.jar
```

The Java command above assumes Java 21+ is on PATH. If using only the local
runtime, replace `java` with `.runtime/java/Contents/Home/bin/java`.
`graphics.c` is compiled as supplied; no `unistd.h` or operating-system sleep
function is used. The drawing library emits `sleep(milliseconds)` commands
which DrawApp plays back.

To retain the command stream and replay it separately:

```sh
./run.sh --commands --delay 25 > build/replay.draw
java -jar drawapp-5.0.jar < build/replay.draw
```

Build messages and search statistics are on stderr, so they do not pollute the
saved drawing commands. Do not redirect stderr into the viewer's input.

## Drawing and controls

The maze and exit are drawn on the background layer. The renderer updates the
foreground robot and presentation statistics when the robot changes. Display
information belongs to the renderer; it is not a discovered map available to
the solver. The solver still sees only the five movement/sensor functions.

The cell key under the maze explains all fills and symbols: white means unvisited,
mint one arrival, lavender two or three, and rose four or more. Blue is the
entrance, gold the exit, and the teal triangle the robot. Teal/amber outlines show
clear/blocked squares immediately ahead. Turning does not increase visits.
For a short demonstration of repeated visits, run:

```sh
./run.sh --preset small --solver iddfs --delay 5
```

The default maze is designed. `--preset large`, `--preset loops`, or an explicit
`--generate WIDTH HEIGHT` invokes generation. Equal seeds and generation settings
reproduce a layout; use another seed to try a variation. Reset replays the same
layout rather than generating another one.

The supplied viewer has these controls:

- **Pause / Play:** pause and resume drawing-command playback.
- **Next / Previous:** step through commands in the retained history. One
  drawing command is not necessarily a complete robot move or animation frame.
- **Reset:** rewind the retained command history; it does not ask the C program
  to generate a new maze or rerun its search algorithm.
- **Close Window:** close the viewer. Ctrl-C in the launching terminal also
  stops a run if the window or pipeline does not respond.

The viewer can buffer commands while the C search continues. A terminal
"solved" message means the C process has found the exit; the window can still
be playing earlier actions. Playback time is affected by the delay, the number
of actions, drawing overhead, and pauses. It is different from search CPU time.
The default solver is wall-following; `--solver iddfs` selects the optional comparison.
Presets are available with `--preset small`, `--preset large`, and `--preset loops`.
The default delay is 50 ms. Start around 50-200 ms for short demonstrations; use a smaller delay for
iterative deepening, which performs many exploratory moves and turns.

If DrawApp reports **History full: older commands are being discarded**, drawing
continues. Previous and Reset are then limited to the retained history. This
warning is not proof that the maze program has failed.

## Verification and packaging

The supplied 5.0 library now builds, and its real Java window has been inspected.
The completed 35x35 view and the cyclic IDDFS animation both display the modern
dashboard correctly. Pause/Play and Reset were checked. A real screenshot is in
`docs/drawapp-preview.png`; the detailed record is in `VALIDATION.md`.

`VALIDATION.md` records which real-library builds, protocol replays, tests, and
live visual checks have actually been completed. Command generation alone does
not verify timing, clipping, foreground clearing, or window appearance; inspect
those in the real viewer.

The source package excludes `graphics.h`, `graphics.c`, the JAR, the Java runtime,
compiled objects, and executables, following the coursework sheet. The marker
supplies the standard course graphics package. The short readMe gives the direct
compile/run command; this setup guide and the developer launcher are not needed
inside the source-only assessed archive.
