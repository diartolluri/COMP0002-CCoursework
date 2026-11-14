# Coursework requirement audit

Reviewed against the seven-page coursework sheet and the Moodle FAQ/instructor
replies supplied in this conversation. This is evidence for the learning/model
solution, not a grade prediction or certification of a student's own work.
PDF page numbers below are page positions in the supplied question sheet.

## Functional specification

| Requirement | Implementation and evidence |
| --- | --- |
| Stages 1 and 2: display a maze and animate the robot (p. 5) | Genuine DrawApp, static background maze and exit, foreground robot; fixed example available by default. A predetermined route is permitted, not required. |
| Stage 3: search from entrance to exit (p. 5) | Default right-wall follower; optional iterative-deepening DFS. Both issue only the five robot operations. |
| Stage 4: algorithmic generation (p. 5) | Randomised recursive DFS carving, one entrance/exit, optional internal loops. `--generate`, `--preset large`, and `--preset loops` invoke it. |
| Stage 5: solve generated mazes (p. 5) | Boundary-to-boundary wall following fits the generator's rules. Generated cyclic/tree cases and maximum-size cases are tested; the default action budget exceeds its documented coarse bound. |
| Rectangular grid of equal squares, x/y coordinates (pp. 4-5) | Dynamically owned grid, x as column and y as row, fixed square cell size within each drawing. |
| One-square outer boundary with one entrance and exit (pp. 4-5) | Validation requires exactly those two openings; generation preserves all other boundary walls. |
| Start on entrance, facing inward; exit marker on exit (p. 5) | Validation infers facing from the entrance edge; robot construction uses this state. Fixtures cover all four sides. |
| Connected entrance and exit (p. 5) | Independent BFS validation rejects unreachable input before animation; validation data never enters the solver. |
| Outside-grid positions count as walls (p. 5) | `mazeTileAt` checks signed bounds before indexing; collision/sensor tests cover every edge. |
| Dead ends and loops (p. 5) | Looped and branching files, generated loops, and exhaustive reachable 5x5 boundary configurations are checked. |
| Whole maze fits the drawing window (p. 5) | Cell size scales to the longest dimension; supported dimensions 5..101. Actual viewer checks cover small, 35x35 and 101x101 mazes; protocol checks also cover extreme rectangles. |
| Unchanging background; foreground animation (p. 4) | One background draw. Visit colours, robot and changing feedback are foreground presentation; actual-command tests reject later background changes. |
| Controllable animation speed (p. 5; FAQ) | Supplied DrawApp `sleep(milliseconds)`; `--delay 0..1000`. Frame sleep portions sum to the requested delay per action. |
| No invalid text sent to DrawApp (p. 6) | Graphics alone use stdout during animation; diagnostics/statistics use stderr. Preview/headless modes are explicit. |

## Robot abstraction

| Operation | Contract and checks |
| --- | --- |
| `forward` | Advances exactly one cardinal square when clear; stays still at a wall. |
| `left` | Anticlockwise 90-degree turn without translation. |
| `right` | Clockwise 90-degree turn without translation. |
| `atMarker` | Integer truth value for the current square's exit marker. |
| `canMoveForward` | Integer truth value for the immediately adjacent square ahead. |

The public `Robot` type is opaque. `solver.c` neither includes the world/internal
robot headers nor calls coordinate, grid or display accessors. The interface
check verifies this at source and object-symbol level. Search frames hold action
history, not a discovered map. Visit counts belong only to rendering. Every
successful forward arrival is followed by exit detection before another action.

## Language, design and source quality

- C17 compiles with standard clang; gcc can be substituted. No extra C libraries,
  platform-specific headers, POSIX file APIs or option-parsing dependencies.
- Supplied graphics.c/h and JAR remain unchanged and are excluded from the ZIP.
- Separate source/header modules; clear owners for allocations; checked failures
  and cleanup; signed coordinates, `size_t` sizes and unsigned masks.
- Standard `stdio.h` file loading/saving and checked command-line numbers.
- AI-assisted code is identified in each application source/header file. Non-trivial
  search/generation references are next to the corresponding implementation.
- The expanded dashboard and optional comparison solver make this reference
  larger than the brief's suggested several-hundred-line scale. That suggestion
  is not a stated hard line limit. Further features should justify their size;
  no requirement calls for every lecture technique to be inserted.

## ReadMe and source package (p. 2)

- `README.md` is a concise plain-Markdown program description and compile/run
  reference, with examples that use only the extracted files and course graphics.
- Standard ZIP named `COMP0002CW1.zip`, containing the final application sources,
  headers, short readMe and six input fixtures; no intermediate implementations.
- Explicit allowlist excludes binaries, debug bundles, Java, supplied graphics,
  launcher, tests, development guides, personal paths and editor configuration.
- Packaging checks integrity and equality with current sources before replacing
  the archive. A separate check extracts it and executes the README's compilation
  command, graphical command output, file input, generation and save/load runs.
- No student name or number is included. This audit and the longer learning
  guides stay outside the assessed-format ZIP.

## Verification and remaining boundaries

Run `make test`, `make sanitize`, and `make package`; see `VALIDATION.md` for
completed results and actual-viewer observations. Launcher tests isolate only
Java process routing; drawing tests use the real supplied C implementation.
The short readMe does not depend on the developer launcher or test tooling.

No finite test suite proves all seeds or all possible environments. Wall
following's argument relies on a reachable boundary exit and inward boundary
start; it is not claimed for arbitrary interior starts or goals. IDDFS has
exponential worst-case work and can stop at its action budget. Limits are
documented instead of being presented as unsolvability or shortest-route claims.

The sheet also requires an assessed submission to be the student's own effort.
Attribution, technical compliance and successful tests cannot establish that
requirement for an AI-assisted reference implementation. Moodle upload and the
submission deadline are administrative actions, not performed by this project.
