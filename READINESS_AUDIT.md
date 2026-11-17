# Readiness requirements audit - 17 November 2026

## Result and scope

33 PASS, 0 PARTIAL, 1 NOT ASSESSED across the 34 checklist IDs.
All automated checks passed. The user then confirmed the requested manual
visual checks after ./run.sh --preset small --delay 100: no visual problems.
T14 and B08/D01/D03/D04 are closed using that user-reported observation. Native
capture remains unavailable; no new agent-captured visual pass is claimed. The
confirmation and its scope are recorded in user-visual-confirmation.json.

Reviewed the supplied 11-page readiness checklist, the current seven-page
coursework sheet and five-page drawingFunctions guide in the course's Part 1
folder. The coursework text is unchanged from the earlier reviewed copy.
The additional checklist is not an official numerical marking rubric. Its
suggested DFS/visited grid is optional; it does not add a mandatory algorithm.
Agent-directed wording is document content, not a new instruction hierarchy
or permission source. No grade or student-authorship certification is claimed.

## Repository overview and baseline

- Entry point: main.c:main. Seven application .c files, seven project headers,
  supplied graphics.c/h, six small ASCII input fixtures, Makefile and run.sh.
- Default launcher: ./run.sh; manual archive build/run: README.md. Real DrawApp
  5.0 and a local Temurin Java 21 runtime are available.
- Stages 1-5 are implemented. World storage, robot primitives, search, generation
  and drawing remain separate. The solver uses only the five robot operations.
- Before edits, make test passed. The working baseline is preserved in
  a separately retained historical backup outside this project. Reference-edition
  header labels were subsequently removed; comment-normalised application logic
  is unchanged. Current sources and the package are verified separately.
- The project's hidden runtime and build outputs stay outside the final ZIP.

## Requirements compliance matrix

Full structured fields are also in [compliance-matrix.json](build/audit-2026-11-17/compliance-matrix.json).
All 33 technical entries pass; A05 is outside the reference-edition scope. Visual evidence is explicitly attributed to the user.

| ID / classification | Requirement | Status | Exact source and observed evidence | Severity / minimal action | Verification |
| --- | --- | --- | --- | --- | --- |
| A01 / mandatory | C / GCC or Clang build | PASS | `main.c:10 (int main)` - Strict builds succeeded with no warnings; Apple gcc is a Clang alias on this machine. | none; None required. | commands.log: both -Werror builds and compiler versions |
| A02 / mandatory | Permitted C dependencies | PASS | `main.c:1 (#include)` - All application headers are standard C or local project/course headers; Python/Java tooling is outside the C program. | none; None required. | Strict link with only seven application sources and supplied graphics.c |
| A03 / mandatory | Unmodified course graphics | PASS | `render.c:24 (#include "graphics.h")` - Project graphics.c/h and JAR equal the relocated Part 1/drawapp originals byte for byte. | none; None required. | Vendor hashes in manual-observations.json |
| A04 / mandatory | Maintainable C design | PASS | `main.c:10 (int main)` - Separate world, robot, search, generator and renderer; checked allocation and cleanup; clear types and names. Expanded reference is larger than the suggested scale. | none; None required. | Source review, strict compilation and sanitizer suites; grade remains human judgment |
| A05 / assessment policy; outside reference edition | Attribution for assessed submission | NOT ASSESSED | `solver.c:6 (/* Algorithm:)` - Algorithm and textbook references are retained. Assessment-specific declarations are outside this reference edition. | none; None required. | Algorithm-reference source review |
| A06 / mandatory | Source-only named archive | PASS | `tools/package.py:7 (names =)` - COMP0002CW1.zip contains exactly 21 current source/header/readMe/maze files and passes integrity checks. | none; None required. | make package; tests/test_package.py |
| A07 / mandatory | Anonymous package | PASS | `tools/package.py:7 (names =)` - No owner name, home path or student identifier found in submitted files; academic author citations are retained. | none; None required. | Archive content scan recorded in manual-observations.json |
| A08 / mandatory | Exact concise build/run instructions | PASS | `README.md:14 (clang -std)` - Actual README compile command succeeds from an extracted copy. Plain Markdown fits one A4 page at 10 pt monospace. | none; None required. | tests/test_package.py; docs/readme-one-page.png; readme-layout.json |
| B01 / mandatory | Rectangular equal-square grid | PASS | `maze.c:129 (int mazeValidate)` - Loader rejects ragged rows/invalid tile values; renderer uses one square size per drawing. | none; None required. | Core/CLI malformed input tests; real-command geometry tests |
| B02 / mandatory | Coordinate convention | PASS | `maze.c:59 (Tile mazeTileAt)` - Signed x/y checked before indexing row y, column x in contiguous storage. | none; None required. | Robot trace/BFS oracle plus 101x5 and 5x101 drawing cases |
| B03 / mandatory | Outer wall and exactly two openings | PASS | `maze.c:129 (int mazeValidate)` - Exactly one non-corner entrance/exit, all other boundary cells walls; independent batch checks agree. | none; None required. | Core boundary enumeration; readiness_audit.py inspect_grid |
| B04 / mandatory | Initial and exit state | PASS | `robot.c:23 (Robot *robotCreate)` - Start uses validated inward-facing entrance; marker occupies the exit. | none; None required. | Four-side robot tests and marker checks |
| B05 / mandatory | Solvable maze | PASS | `maze.c:74 (static int exitReachable)` - Independent Python BFS agrees on all 20 added generated cases; malformed unreachable files are rejected. | none; None required. | Core BFS oracle; independent generated_cases evidence |
| B06 / mandatory | Outside treated as wall | PASS | `maze.c:59 (Tile mazeTileAt)` - All four boundary orientations reject outward motion and preserve pose. | none; None required. | tests/test_core.c testRobot |
| B07 / mandatory | Loops and dead ends | PASS | `solver.c:139 (static SearchResult followWall)` - All 18,676 reachable 5x5 configurations and independent fixtures solve. Wall-following is justified only for the specified boundary conditions. | none; None required. | Core exhaustive test; looped.txt reachable cycle; branching.txt has two dead ends |
| B08 / mandatory | Visible complete maze | PASS | `render.c:378 (int renderInit)` - 101x101/101x5/5x101 command bounds pass. Earlier real-window captures exist; fresh capture is blocked by the system. User confirmed the requested small-maze visual checks on 17 November: no visual problems. | none; None required. | Geometry checks PASS; repeat native fit check when capture works; user-visual-confirmation.json (user-observed, not agent-captured) |
| C01 / mandatory | Five specified controls | PASS | `robot.h:4 (typedef struct Robot)` - Opaque Robot exposes exactly forward/left/right/atMarker/canMoveForward to search. | none; None required. | check_solver_interface.py source and nm guard |
| C02 / mandatory | Forward semantics | PASS | `robot.c:53 (void forward)` - Moves one cardinal square when clear; both inner walls and outer boundaries preserve position/heading/counters when blocked. | none; None required. | Explicit inner-wall regression and four outer-edge tests |
| C03 / mandatory | 90-degree rotations | PASS | `robot.c:64 (void left)` - Four equal turns and mixed turns restore pose; no translation. | none; None required. | testRobot and per-action trace assertions |
| C04 / mandatory | Read-only front sensor | PASS | `robot.c:46 (int canMoveForward)` - False at inner walls/outside; true for adjacent passages; querying preserves pose and counters. | none; None required. | New explicit purity assertions in testRobot |
| C05 / mandatory | Read-only current-cell marker | PASS | `robot.c:78 (int atMarker)` - False beside the exit, true after entering; sensor preserves pose and counts. | none; None required. | Explicit check after every corridor step, including adjacent exit |
| C06 / mandatory | Coherent state/drawn heading | PASS | `robot.c:4 (struct Robot)` - State updates are traced; actual triangles point east initially, south after right(), and east again after left(). | none; None required. | Logical trace checks plus real-library renderer-probe polygon assertions |
| D01 / mandatory | Draw maze | PASS | `render.c:135 (static void drawBackground)` - Genuine-library wall/passage/key commands pass; previous screenshots show layout. Fresh native capture unavailable. User confirmed the requested small-maze visual checks on 17 November: no visual problems. | none; None required. | Protocol checks PASS; native display needs repeat; user-visual-confirmation.json (user-observed, not agent-captured) |
| D02 / mandatory | Static background | PASS | `render.c:135 (static void drawBackground)` - Window sized once, then one background draw; later state is foreground only. | none; None required. | test_drawapp.py check_layers |
| D03 / mandatory | Directional foreground robot | PASS | `render.c:317 (static void drawRobotAt)` - Foreground triangle position and rotation pass numerical checks; fresh perceptual inspection unavailable. User confirmed the requested small-maze visual checks on 17 November: no visual problems. | none; None required. | Renderer-probe orientation PASS; native legibility needs repeat; user-visual-confirmation.json (user-observed, not agent-captured) |
| D04 / mandatory | Animate entrance to exit | PASS | `render.c:339 (static void drawAction)` - Logical traces and frame/sleep commands pass, including stopping on first marker arrival. Earlier live playback was observed. User confirmed the requested small-maze visual checks on 17 November: no visual problems. | none; None required. | Core/DrawApp tests PASS; fresh ghosting/playback observation blocked; user-visual-confirmation.json (user-observed, not agent-captured) |
| D05 / mandatory | Correct DrawApp API | PASS | `render.c:135 (static void drawBackground)` - Real supplied API compiles; setWindowSize precedes drawing; limits checked against 50x50..2500x1500. | none; None required. | Actual drawingFunctions.pdf reviewed; protocol/layer tests |
| D06 / mandatory | Perceptible bounded timing | PASS | `render.c:339 (static void drawAction)` - Default 50 ms, configurable 0..1000; each action retains its exact requested sleep total; output bound checked. | none; None required. | DrawApp smooth-frame timing/protocol cases |
| D07 / mandatory | Safe stdout protocol | PASS | `main.c:27 (fprintf(stderr)` - Drawing alone uses stdout; diagnostics use stderr; API message supplies result panel. | none; None required. | All real-library runs parse as valid drawing protocol |
| E01 / stage 3 | Genuine search | PASS | `solver.c:139 (static SearchResult followWall)` - Default wall-following and optional IDDFS respond to sensors, not fixed routes. | none; None required. | Unseen generated cases, independent fixtures and exhaustive boundary cases |
| E02 / stage 4 | Algorithmic generation | PASS | `generator.c:87 (int mazeGenerate)` - Recursive backtracking produces connected layouts; seeds repeat; optional loops preserve invariants. | none; None required. | 20 additional cases yield 19 layouts; core tests cover 20 seeds x 3 loop settings |
| E03 / stage 5 | Solve generated and independent mazes | PASS | `solver.c:139 (static SearchResult followWall)` - All additional cases solve within 20*width*height actions, including 101x101; independent cyclic and dead-end fixtures also solve. | none; None required. | evidence.json and existing fixture/core tests |
| E04 / recommended; suggested data structure optional | Revisits and physical backtracking | PASS | `solver.c:50 (static int returnToParent)` - IDDFS restores failed-branch pose; renderer tracks arrivals/revisits. Optional solver visited-grid suggestion is deliberately not adopted. | none; None required. | Pose-restoration, budget, visitation and stop-at-exit tests |
| E05 / recommended | Bounded understandable runtime | PASS | `solver.c:23 (static int perform)` - Actions have explicit resource budgets; budget stops do not claim no route; zero-delay/headless testing is supported. | none; None required. | Time-bounded suites; 20 batch runs bounded by 20WH |

## Minimum test matrix

| ID | Status | Observed verification |
| --- | --- | --- |
| T01 | PASS | Automated startup tests plus user confirmation of the visible inward-facing start. |
| T02 | PASS | Explicit inner-wall forward attempt preserves position, direction and counts. |
| T03 | PASS | Four boundary-facing tests reject leaving the grid. |
| T04 | PASS | Four left/right and mixed turns; actual triangle rotation checked geometrically. |
| T05 | PASS | Straight corridor reaches marker; no action after first arrival. |
| T06 | PASS | Multiple-turn fixtures, generated mazes and per-action traces. |
| T07 | PASS | Independent branching.txt has two reachable dead ends; both solvers complete. |
| T08 | PASS | Independent looped.txt has a reachable cycle; both solvers complete. |
| T09 | PASS | Adjacent marker is false; after entering it true; sensor calls preserve state. |
| T10 | PASS | 5..101 geometry checks, earlier large-window observations and user confirmation that the current small maze fits. |
| T11 | PASS | 20 extra reproducible seed/size/loop cases, 19 distinct layouts; independent BFS and boundary validation. |
| T12 | PASS | Generated and independent cycles/dead ends; explicit 20WH batch action bounds. |
| T13 | PASS | CLI malformed numbers/options/files; launcher option-like filenames and failures. |
| T14 | PASS (user-confirmed) | User reported no visual problems for the requested full-layout, initial-heading, ghosting and exit-stop checks. Native agent capture still fails. |
| T15 | PASS | Exact named source ZIP; extracted README build/run; anonymous scan; one-page readMe layout checked. |

## Prioritised findings: ten observations, not ten invented defects

No P0/P1 application defect was reproduced. Where existing behaviour already
passes, the smallest correction is to leave it working.

| Priority / status | Finding and impact | Exact location | Smallest correction / regression verification |
| --- | --- | --- | --- |
| P4 / CLOSED | Agent capture failed; manual visual acceptance was supplied by the user. | user-visual-confirmation.json; D01/D03/D04/B08 | User confirmed no visual problems after the requested small-maze run; capture failure is retained as a tooling limitation. |
| P4 / CLOSED | Inner-wall blocked movement previously lacked a targeted direct primitive assertion. | tests/test_core.c:testRobot | Added a wall directly ahead; assert unchanged pose and counters after sensor query and forward. |
| P4 / CLOSED | Sensor purity and adjacent-marker behaviour needed explicit assertions. | tests/test_core.c:testRobot | Check pose/counters after queries and marker result after every corridor step. |
| P4 / CLOSED | Protocol tests counted polygons but lacked explicit drawn-heading assertions. | tests/test_drawapp.py:probe_renderer | Assert east/south/east triangle orientation and same-cell bounds after right/left. |
| P4 / CLOSED | Window test limits did not mirror the actual documented DrawApp contract. | tests/test_drawapp.py:protocol | Use 50x50..2500x1500; all existing/max-size streams pass. |
| P4 / CLOSED | Existing 20-seed core cases were mostly 7x7; add varied larger rectangular seed evidence. | tools/readiness_audit.py | 20 independent validations/solutions through 101x101, explicit time/action bounds and repeatability. |
| P4 / CLOSED | One-page readMe had a word bound but no page-layout evidence. | README.md; docs/readme-one-page.png | Render plain Markdown on A4, 10 pt monospace: 636 pt of 745.89 pt height, visually checked. |
| P4 / CLOSED | Course source files moved to Part 1; old setup path was stale. | DRAWAPP_SETUP.md | Updated the path and rechecked all vendor bytes against relocated originals. |
| INFO / CHECKED | The new checklist suggests visited-grid DFS; it is not a compulsory replacement for a working solver. | solver.c:followWall; DESIGN.md | Retain the stricter map-free interface and document boundary assumptions, performance and finite-test limits. |
| INFO / CHECKED | A good technical artifact does not establish student ownership or an 80+ mark. | Source attributions; README.md | Retain algorithm references and reference-project status; assessed-submission declarations are not evaluated here. |

## Minimal-change plan and disposition

1. Preserve the working baseline and run existing tests - completed before changes.
2. Close diagnostic coverage gaps in testRobot/probe_renderer/protocol - completed;
   relevant tests and sanitizers pass. The application algorithms are unchanged.
3. Add reproducible varied-size evidence and exact ID mapping - completed; logs,
   JSON cases and this report are outside the final archive.
4. Verify the real vendor files, readMe page layout and extracted package - complete.
5. Obtain manual visual acceptance - completed by the user: no visual problems.
   This is user-confirmed evidence, not a claim of new agent window capture.

No optional solver rewrite or unrelated feature is required by the evidence.

## Verification log and reproduction

```sh
python3 tools/readiness_audit.py --output build/audit-2026-11-17
```

The actual commands, complete stdout/stderr and exit codes are in
[commands.log](build/audit-2026-11-17/commands.log). It records:

- Fresh -std=c17 -Wall -Wextra -Wpedantic -Werror graphical compilation with an
  explicit actual source list, through clang and the gcc driver. Both are Apple
  Clang 21 here; this is not falsely counted as an independent GNU GCC test.
- make test: core robot/search/generator/file checks, 18,676 reachable exhaustive
  5x5 cases, CLI and architecture guards, real graphics protocol and launcher.
- make sanitize: AddressSanitizer/UndefinedBehaviorSanitizer for core, CLI and
  genuine-library graphical/probe cases.
- make package: fixed allowlist, current-source equality and ZIP integrity;
  extracted README compile command, graphical output, file loading and save/load.
- 20 additional generated cases: seed, size, loop setting, SHA-256, independent
  BFS distance, topology and observed moves/turns/actions are in evidence.json.
- Vendor SHA-256 and the capture failure and user visual confirmation are in manual-observations.json.

### Additional generator results

| Seed | Size | Loops % | Reachable/open cells | Shortest distance | Observed actions | Result |
| --- | --- | --- | --- | --- | --- | --- |
| 0 | 5x5 | 0 | 9/9 | 6 | 30 | PASS |
| 1 | 7x9 | 25 | 28/28 | 12 | 72 | PASS |
| 2 | 15x11 | 50 | 84/84 | 22 | 126 | PASS |
| 3 | 21x15 | 100 | 195/195 | 32 | 96 | PASS |
| 4 | 35x35 | 0 | 579/579 | 182 | 1506 | PASS |
| 5 | 101x5 | 25 | 214/214 | 130 | 558 | PASS |
| 6 | 5x101 | 50 | 225/225 | 122 | 414 | PASS |
| 7 | 101x101 | 100 | 7402/7402 | 198 | 594 | PASS |
| 8 | 5x5 | 25 | 9/9 | 6 | 30 | PASS |
| 9 | 7x9 | 50 | 29/29 | 12 | 36 | PASS |
| 10 | 15x11 | 100 | 95/95 | 22 | 66 | PASS |
| 11 | 21x15 | 0 | 141/141 | 92 | 420 | PASS |
| 12 | 35x35 | 25 | 654/654 | 66 | 246 | PASS |
| 13 | 101x5 | 50 | 225/225 | 118 | 426 | PASS |
| 14 | 5x101 | 100 | 250/250 | 102 | 306 | PASS |
| 15 | 101x101 | 0 | 5001/5001 | 2234 | 10890 | PASS |
| 16 | 5x5 | 50 | 10/10 | 6 | 18 | PASS |
| 17 | 7x9 | 100 | 31/31 | 12 | 36 | PASS |
| 18 | 15x11 | 0 | 71/71 | 58 | 210 | PASS |
| 19 | 21x15 | 25 | 158/158 | 32 | 132 | PASS |

## Abstraction and integrity interpretation

The original brief specifies the five robot actions/sensors; it does not
unambiguously prohibit all higher-level knowledge or prescribe a visited grid.
The supplied lecturer clarification advises using just those functions and says
a map is unnecessary. This project deliberately exposes a stricter opaque API:
solver frames hold action history, while the renderer alone owns visit counts.
Keeping this design is compatible with the functional requirements. Counters,
control state and physical backtracking do not constitute a hidden world map.

Algorithm and textbook references remain in the application sources. This is a
learning/model reference edition. Assessment-specific ownership declarations
are outside its scope, so A05 is recorded as NOT ASSESSED.
There is no guarantee of 80+ and no invented numerical allocation per stage.

## Final packaging checklist

- [x] Exact filename COMP0002CW1.zip; standard ZIP, integrity checked.
- [x] Exactly 14 application source/header files, one short readMe and six maze data files.
- [x] No binaries, object/debug files, tests, logs, audit reports or developer scripts.
- [x] No vendor graphics.h/c, JAR, local Java runtime, personal paths/name/number.
- [x] ReadMe's direct compile and run commands work from a clean extracted copy.
- [x] ReadMe fits one A4 page in the recorded plain-Markdown print layout.
- [x] Packaged application files equal the current working versions.
- [x] Algorithm references preserved; A05 is not assessed for this reference edition.
- [x] Manual visual acceptance supplied by the user; agent capture failure remains documented.

The final source ZIP remains in build/COMP0002CW1.zip. Baseline, diagnostics,
tests, page preview and this report are intentionally kept outside it.
