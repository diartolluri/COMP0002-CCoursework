# Validation record

14 November 2026; Apple clang 21.0.0, arm64 macOS, C17.

## Executed checks

- `make test`: passed. The C suite covers robot turns/collisions/outside-grid
  behaviour on all four boundaries; search traces checked one action at a time;
  independent BFS shortest-distance comparisons; all four initial headings;
  unsuccessful-search pose restoration; budgets 1..80; an unreachable cyclic
  test world; 60 generated small-maze searches with different seeds/loop settings;
  deterministic generation; connectivity/tree/cycle invariants; dimensions up to
  101x101; file roundtrips; and all six hand-designed fixture files.
- The executable suite passed: valid fixtures, independent grid validation,
  generated/save/load roundtrips, CRLF and final-no-newline handling, malformed
  files and numbers, invalid/missing options, save failures, and resource errors.
- The interface check compiled `solver.c` separately and used `nm` to confirm
  its only application dependencies are `forward`, `left`, `right`, `atMarker`,
  and `canMoveForward`. Other imports are allocation functions.
- `make sanitize`: core and executable checks passed with AddressSanitizer and
  UndefinedBehaviorSanitizer, with compiler warnings treated as errors.
- `make package`: produced a ZIP containing 21 source/header, readMe and fixture
  files. Verified archive integrity and its explicit allowlist. Extracted it to
  a temporary directory, compiled with strict C17 warnings, and solved the cyclic
  fixture from that isolated directory.
- `run.sh --preview` worked when launched from `/tmp`; VS Code JSON parsed.
- Missing graphics files cause `make draw` to fail with an explicit setup message.

The generator was also checked independently during implementation on 400
deterministic generation cases through 101x101 with loops 0/25/50/75/100 under
address/undefined-behaviour sanitizers.

## Earlier IDDFS comparison runs

These measurements use IDDFS, seed 42 and the two-million-action guard.
Wall-following is now the default; select `--solver iddfs` to reproduce this table.

| Grid | Loop percentage | Result | Actions | Successful depth |
| --- | --- | --- | --- | --- |
| Default 9x7 | Fixed fixture | Solved | 1,306 | 16 |
| 15x11 | 0 | Solved | 8,270 | 38 |
| 15x11 | 15 | Solved | 8,270 | 38 |
| 21x21 | 0 | Solved | 27,242 | 66 |
| 21x21 | 15 | Solved | 594,662 | 38 |
| 31x31 | 0 | Solved | 52,250 | 90 |

A percentage is a probability for each eligible wall, not a promise that this
fraction of all cells will open. Identical results for a particular low-percentage
seed are possible. Total animated motion includes backtracking, not just the
successful path depth. Large/dense cyclic cases may exceed the guard; no claim is
made that every supported size/seed finishes within the default budget.

## DrawApp 5.0 integration and visual verification

The genuine graphics.h, graphics.c and drawapp-5.0.jar supplied in the course's
drawapp directory are now copied into the project and verified byte-identical.
An official Temurin Java 21.0.13 runtime was downloaded, checked against its
published SHA-256, and extracted into the ignored project-local .runtime folder.
No system-wide Java settings or supplied graphics code were changed.

- `make draw` compiles the real graphics implementation successfully.
- `tests/test_drawapp.py` compiles against that implementation and checks the
  actual command stream, window bounds, one static background, foreground use,
  clean stdout, live/final dashboard values, visitation accounting and exact sleep
  totals, including smooth movement split across subframes.
- Tested graphical streams include a 5x5 maze, the cyclic fixture, a 35x35 maze
  with a budget stop, and an early three-action resource stop. The largest of
  these protocol tests contains 334,152 commands; the check caps record size at
  20 MB to detect runaway drawing output in those fixtures.
- Replayed actual emitted commands through the supplied Java JAR and inspected
  its window. A 35x35 generated maze completed with 250 moves and 500 turns.
  The cyclic IDDFS example was observed during playback and paused at 68 moves,
  245 turns, 313 actions, pass/depth 6, and 13/43 visited open cells.
- Checked Pause/Play, Reset to 0 commands, and replay to completion in the real
  viewer. Reset/Previous operate on drawing commands, not complete robot moves.
- The first visual inspection found bottom clipping from the viewer's built-in
  controls. The compact 816 by 622..644 canvas was then checked again: the maze,
  complete dashboard, legend and playback controls are all visible.
- Saved the real-viewer screenshot in docs/drawapp-preview.png. The viewer was
  left open, paused on the cyclic IDDFS demonstration for inspection.

`make test` (including real-library protocol tests), `make sanitize`, and
`make package` all passed after integration. The archive still excludes DrawApp,
Java, development guides, test tools and binaries. Standard C-only headless
testing remains separate from the graphical launcher.

Runtime download source and checksum metadata:
https://adoptium.net/installation/ci-scripts and the Adoptium Java 21 assets API.
This is development/runtime tooling, not a third-party C library.

This record is evidence of the checks above, not a grade prediction or a guarantee
against every possible defect. See TEST_PLAN.md for the broader acceptance plan.

## Improvement verification

The default solver is now wall-following. The five-operation public robot API
and its import guard remain unchanged. All presets and explicit override ordering
are covered by CLI tests. The permanent core suite now enumerates 67,584 possible
5x5 configurations and verifies that validation agrees with an independent BFS;
all 18,676 reachable configurations are solved by wall-following. This includes
cyclic interiors, adjacent boundary openings and all edge orientations. Generated
cyclic cases, all fixture files and maximum-size tree mazes are also checked with
the wall solver. Traces still reject any action after the first exit arrival.

Real-library protocol checks verify that static dashboard labels are emitted only
once, final counts are correct, playback sleeps retain exact totals, and output
stays within a per-action command bound. For the same zero-delay cyclic IDDFS
fixture, output fell from 334,152 to 77,308 commands (approximately 77% fewer).
At 20 ms/action, the 35x35 seed-42 wall demonstration emits 47,832 commands,
compared with the earlier 84,100-command zero-delay capture even before allowing
for the new run's sleep commands and interpolation.

`make test`, `make sanitize`, `make draw` and `make package` passed after these
changes. The real viewer was checked with `--preset large --delay 20`: sensor
outline, clear/blocked badge, decision banner, revisit count and priority rule
are visible without clipping. At a paused observation it showed 79 moves,
159 turns, 238 actions, 68 visited cells and 12 revisits. Numeric values can lag
an individual animation frame by the documented refresh interval. A refreshed
real-viewer screenshot is saved in docs/drawapp-preview.png.

The live decision banner is set by the algorithm before issuing its action.
The front-cell preview is display-only world information; it neither supplies
new observations nor a map to the solver. The completion message adds new-cell
rate and producer CPU time, explicitly distinct from commanded playback time.

## Cell key and arrival colours

The renderer now keeps display-only arrival counts, saturated at four. Its
two-row key distinguishes unvisited white, mint one visit, lavender two–three,
rose four-plus, blue entrance, gold exit, robot and front-sensor outlines.
Turning leaves counts unchanged; the initial snapshot counts the entrance once.

After this change, `make test`, `make sanitize` and `make draw` passed. The
renderer probe deliberately enters the same passage more than four times and
checks the actual tile-fill commands for all four bands, count saturation,
turn invariance, unique-cell accounting and exact sleep totals. The dashboard
tests also require every key entry. The cyclic IDDFS protocol fixture now emits
82,910 commands; extra tile counts/key drawing account for its increase from
the previous 77,308, still below the per-action bound.

The real DrawApp viewer completed the 35x35 seed-42 wall demonstration at zero
delay: 250 moves, 500 turns, 179/579 unique open cells and 72 revisits. The entire
816x664 canvas, both legend rows and viewer controls fit without clipping. The
refreshed screenshot is saved in `docs/drawapp-preview.png`. The supplied
graphics.h, graphics.c and JAR were rechecked against the original files and
remain byte-identical.

The documented `--preset small --solver iddfs --delay 5` demo also completed in
the real viewer: 262 moves, 1,044 turns, 19/26 unique open cells, 244 revisits and
6.53 seconds of commanded playback. Mint, lavender and rose fills, their numeric
labels, the blue entrance and gold exit are visible together. Both key rows fit.
This view is saved in `docs/drawapp-visit-key.png` and left open for inspection.
`make package` then rebuilt and verified the 21-file source archive with the
updated renderer, comments and short readMe.

## Final polish and requirement audit

The coursework sheet was reread in full. `REQUIREMENTS_AUDIT.md` maps the maze,
robot, graphics, language, documentation and packaging requirements to code and
evidence, with the own-work requirement and supported limits stated separately.

- Options now parse in one pass; explicit values override preset defaults in
  either order. Regression cases cover dimensions, seeds and loop overrides.
- Launcher routing skips option values. Tests use real C executables and a
  test-only Java process to verify normal, preview, headless and command modes,
  literal option-like filenames, and failure statuses for malformed input.
- Shared named directions replace numeric headings in the world model. Unused
  renderer state was removed; comments and source/header attribution were cleaned
  up. BFS, IDDFS, wall-following, generator and LCG references are recorded.
- `make test` passed, retaining all 18,676 reachable exhaustive 5x5 cases and the
  five-operation solver guard. Real-library drawing checks now include 101x101,
  101x5 and 5x101, each with a deliberate 25-action guard.
- `make sanitize` passed for core, CLI and genuine-library graphical/probe tests
  under AddressSanitizer and UndefinedBehaviorSanitizer.
- Clang static analysis reported no diagnostics for all seven final application
  translation units, with graphical code enabled.
- `make package` rebuilt a 21-file, 21,157-byte archive. Its fixed allowlist,
  integrity and byte-for-byte equality with current sources passed. The separate
  package check extracted it, executed the README compilation command against
  the course graphics, and exercised graphical output, help, file input,
  generation and save/load from that isolated directory.
- The 321-word README uses direct executable commands and has no dependencies
  on the excluded launcher, Java runtime folder or development guides.
- The supplied graphics.c/h and JAR remain byte-identical to their originals.

The real viewer was also checked at 101x101 with a 25-action stop: full maze,
exit, key, dashboard and native controls fit without clipping. That image is in
`docs/drawapp-maximum.png`. The stopped result correctly reports a resource limit
rather than no route; the tiny cells at this size naturally make the robot small.

The final 35x35 seed-42 run was observed to completion at 10 ms/action: 250 moves,
500 turns, 750 actions and 7.50 seconds of commanded playback, with 179/579 open
cells reached and 72 revisits. Pause and Play were checked during playback. The
entire key, dashboard, exit and native controls remain visible; the refreshed
`docs/drawapp-preview.png` records this final build. The completed viewer is left
open. The source package remains byte-identical to the final application files.

## 17 November readiness checklist verification

The additional eleven-page requirements/audit PDF was reviewed against the
current seven-page coursework and five-page DrawApp API guide in Part 1. The
original coursework text is unchanged. READINESS_AUDIT.md records all 34 IDs,
T01-T15, exact references, findings, corrective actions and verification evidence.

The existing program passed its baseline tests before changes. The application
.c/.h files are unchanged in this pass; a working baseline is saved outside the
source ZIP. Tests now explicitly check inner-wall collisions, read-only sensors,
adjacent-exit detection, rendered east/south/east headings, same-cell turning,
and the actual 50x50..2500x1500 DrawApp window limits.

`python3 tools/readiness_audit.py --output build/audit-2026-11-17` passed. It
records fresh strict -Werror graphical builds through clang and the gcc driver;
both are Apple Clang 21 here, not two independent compiler implementations.
`make test`, `make sanitize` and `make package` passed. Twenty additional
seed/size/loop cases through 101x101 produced 19 distinct layouts, all independently
validated, reproducible and solved within their explicit 20WH action bounds.
The largest observed count in that batch was 10,890 actions. JSON and complete
command/output logs are in build/audit-2026-11-17, outside the final archive.

The README was rendered and visually inspected as plain Markdown on A4 using
10 pt monospace and 12 pt line height: 636 pt content height fits the 745.89 pt
text area. docs/readme-one-page.png records the full one-page layout. The source
ZIP was extracted and built from the README command, checked against current
sources, scanned for personal paths/name and verified to exclude development
files and vendor assets. Project graphics.c/h/JAR equal the relocated originals.

A fresh native visual inspection could not be completed: the Java wrappers
reported cgWindowNotFound, and accessing Finder also failed with macOS
ScreenCaptureKit SCStreamErrorDomain -3811 (audio/video capture failure).
At that point T14 was NOT TESTED, and B08/D01/D03/D04 were PARTIAL in the
matrix. Earlier real-viewer observations/screenshots remain
available and are not claimed as a newly observed pass. User confirmation of
the visible entrance, turn, complete layout and exit stop has been requested.

The user subsequently answered "no visual problems" to the requested checks
after `./run.sh --preset small --delay 100`: full maze fit, inward-facing start,
clean movement/turns and a stop on the gold exit. This closes T14 and
B08/D01/D03/D04 as user-confirmed visual evidence. READINESS_AUDIT.md now records
34 PASS and zero PARTIAL entries. The confirmation is preserved in
build/audit-2026-11-17/user-visual-confirmation.json. Native agent capture remains
unavailable; these observations are attributed to the user.

## Reference-edition label cleanup

At the user's request, descriptive creation labels were removed from all 14
application source/header files and the current introductory documentation.
Algorithm and textbook references remain. Comparison with the separately
preserved baseline confirms unchanged application logic after ignoring comments
and whitespace. Earlier snapshots and their historical diff are stored outside
the active project in /private/tmp/c-coursework-reference-label-backup-2026-11-17.

`make test` and `make package` passed after cleanup; their logs are in
build/reference-refresh-tests.log and build/reference-refresh-package.log.
The rebuilt source ZIP matches current files and contains no creation labels.
The README page preview was regenerated. The reference-edition audit now has
33 technical PASS entries; assessment-specific attribution item A05 is recorded
as NOT ASSESSED, rather than retaining an inaccurate submission-compliance claim.
