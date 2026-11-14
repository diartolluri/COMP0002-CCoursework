# Verification and assessment checklist

This development document describes checks for the reference/model solution.
It is not a claim that every check has already passed. Keep the submitted readMe
short; exclude this document from an assessed package.

## What earns credit

The coursework sheet rewards working functionality, appropriate use of C,
cohesive functions and data structures, readable naming/layout, suitable design,
and comments that add information. It explicitly prefers clean, straightforward
working code to poorly organised complexity. Completing all five stages supports
a stronger solution but cannot guarantee a particular mark.

| Requirement | Useful evidence |
| --- | --- |
| Stage 1: display a maze | Inspect a complete maze and exit in the real DrawApp window. |
| Stage 2: animated robot | Observe movement, turns, facing direction, collision behaviour, and the final stop. |
| Stage 3: search | Solver source uses the five robot operations; it solves branching and dead-end fixtures. |
| Stage 4: generation | Independently validate generated mazes across dimensions and seeds. |
| Stage 5: generated-maze coverage | Explain search completeness and resource limits; exercise cyclic as well as tree-like cases. |
| C and design quality | Warning-free builds; small public interfaces, private helpers, explicit allocation ownership, and meaningful tests. |
| Portability | Standard C library and unmodified DrawApp files only; no operating-system headers or external runtime libraries. |

## Search argument and its limits

The intended general search is iterative-deepening depth-first exploration of
physical action paths. Search sees an opaque robot and only `forward`, `left`,
`right`, `atMarker`, and `canMoveForward`. It receives no world coordinates,
maze dimensions, visited-cell map, or generated solution.

The important invariant is: after an unsuccessful branch, the robot returns to
the caller's square facing exactly the direction it faced when that branch
started. Turning back for the return move is not enough; restore the caller's
orientation as well. Every unsuccessful complete depth-limited search must
restore the initial state before the next depth iteration.

At finite depth there are finitely many paths. Increasing the limit eventually
includes a path to a reachable exit in a finite valid maze. Omitting an immediate
reverse move below the root is safe because a shortest route does not contain
such a pair; root exploration must still consider every available direction.
Cycles therefore cannot create an unbounded single depth-limited branch.

This is a completeness argument with unbounded resources, not a fast-running
guarantee. On mazes with many cycles the number of explored paths can grow
exponentially. Time, action, depth, and memory limits must produce a distinct
resource-exhaustion result, never a false report that no route exists. A general
graph algorithm such as A* or map-based BFS would not respect this robot's
restricted knowledge. Independent world validation may use BFS: it is not the
robot solver.

## Automated checks

Build the normal program and a separate development build with
`-fsanitize=address,undefined`, warnings enabled, and debug symbols. Do not make
sanitizer libraries a runtime requirement for the normal executable. Recompile
from clean sources and verify header changes rebuild dependants.

### Robot contract

- Four left turns and four right turns restore the starting orientation.
- A left turn followed by a right turn is neutral; every turn preserves position.
- A clear forward move advances exactly one grid square in the facing direction.
- A blocked forward call changes neither position nor direction.
- North/south/east/west boundaries act as walls, including outside entrance and
  exit openings. Check the entrance on each of the four sides.
- `atMarker` is false at the entrance and true on the exit; state remains valid
  after a blocked attempt to leave the exit.
- Read-only sensors neither move the robot nor trigger redraw/state changes.
- Validate initial direction and position rather than indexing direction arrays
  with arbitrary invalid values.

### Maze and generator

- Test minimum and maximum supported dimensions, mixed dimensions, and rejected
  out-of-range dimensions. If generation requires odd dimensions, reject even
  sizes explicitly rather than silently changing them.
- For each generated grid, count exactly one entrance and exit, both on the
  boundary and not corners. All other boundary squares must be walls.
- Run an independent traversal from the entrance and verify the exit is
  reachable. For a generator that claims all passages are connected, check every
  passable square too.
- For zero extra loops, verify the passable graph is connected and has
  `edges = vertices - 1`; for looped output verify cycle creation in suitable
  nontrivial dimensions, while not demanding a cycle when no removable wall
  exists.
- The same size, seed, and loop setting should produce identical output under
  the project's deterministic random-number generator. Different seeds should
  produce varied layouts over a batch; do not assume every pair must differ.
- Save/load round trips preserve dimensions, tiles, entrance, exit, and initial
  facing. Run validation again after loading.

### Input and command-line errors

- Exercise no options, help, every supported mode, and representative combinations.
- Reject unknown options, missing values, conflicting modes, and unexpected
  positional arguments with a nonzero status and a useful stderr message.
- Test numeric zero, negative values where forbidden, leading signs, huge values,
  trailing junk, and empty arguments. Check conversion overflow before narrowing.
- Test nonexistent/unreadable input and unwritable output paths without crashes.
- Reject empty, nonrectangular, overlong, oversized, invalid-character, embedded
  NUL, missing-marker, duplicate-marker, extra-boundary-gap, disconnected-exit,
  and invalid-entrance mazes.
- Accept documented empty-tile representations, preserve spaces, accept CRLF,
  and handle a final line without a newline. Check `ferror` separately from EOF.
- Text preview and help may use stdout. Graphics mode must not mix human-readable
  status, build messages, or errors into the DrawApp command stream.

### Solver behaviour

- Solve straight corridors, both turn directions, several dead ends, asymmetric
  branching, looped rooms, and exits on every boundary side.
- Include a very short route whose exit is reached before a branch's depth limit;
  verify no additional motion or backtracking occurs after success.
- Capture the actual movement trace in tests through a renderer/observer callback.
  Independently check every transition against the fixture grid, without exposing
  that grid to the solver.
- For a small test graph, compare the successful IDDFS depth with an independent
  shortest-path length when the implementation claims shortest movement depth.
  Total animation moves include failed iterations and are not a shortest route.
- Use a deliberately tiny search budget to exercise resource exhaustion and
  cleanup. Check correct status/output and no memory leaks. Test a budget reached
  during a turn, a move, and backtracking if those are separate counted actions.
- Generate a fixed collection of zero-loop and cyclic mazes. Record seeds and
  parameters for any failure so that it can be reproduced. A batch of passing
  seeds is evidence, not a general correctness proof.
- Audit includes and signatures: `solver.c` must not include a world/internal
  robot header or call anything that leaks coordinates, map tiles, or dimensions.

### Memory and arithmetic

- Check allocation return values and free all frame nodes on success and failure.
- A maze owns its cell buffer; copying its pointer must not create a second owner.
- Check dimensions and allocation-size multiplication before allocating; bounds
  checks must happen before indexing, including negative coordinates.
- Use unsigned types/literals for masks and valid shift counts. Test mask truth
  with a nonzero comparison, rather than comparing every masked result to 1.
- Use type-correct format specifiers and never pass a freed object to a callback.

## Real graphics acceptance

These checks require the original Moodle graphics files, the DrawApp JAR, and a
compatible Java runtime. A successful compile or a text-mode run cannot certify
the appearance or buffering of the real Java drawing application.

1. Compile/link against the genuine unmodified graphics files.
2. Start the documented pipeline and inspect the entire maze at small and large
   supported sizes. All cells, margins, and the exit must fit in the window.
3. Confirm the background is drawn once, foreground clearing leaves no robot
   trails, the robot's facing direction is visible, and the exit stays visible.
4. Try delays of 50, 100, and 200 milliseconds. The animation should remain usable
   without overwhelming the DrawApp command buffer; zero delay is a testing mode.
5. Finish at the marker and stop issuing movement instructions. Confirm final
   drawing/status behaviour and no invalid-command warnings from extra stdout.
6. Check the actual API declarations for layers, clearing, polygon arrays,
   colours, and `sleep(milliseconds)`; do not invent replacement graphics APIs.
7. Check the complete two-row key: white unvisited, mint one arrival, lavender
   two–three, rose four-plus, blue entrance, gold exit, robot and sensor outlines.
   The renderer probe must exercise every passage fill, preserve counts across
   turns, saturate beyond four, and retain exact movement/turn sleep totals.
8. Check 101x101, 101x5 and 5x101 command geometry, including a resource stop.
   The renderer should keep the full grid, key, status and counters within bounds.
9. Check launcher routing with option-like filenames. A filename supplied after
   `-f` or `--save` must not be interpreted as `--headless`, `--preview`, or
   `--commands`; malformed options must still return an unsuccessful status.

## Final package audit

- One standard ZIP named `COMP0002CW1.zip`, with source code, required data/images,
  and the short readMe. No name or student number inside.
- No binaries, object/dependency files, debug bundles, IDE/virtual-environment
  directories, development notes/tests, or supplied DrawApp files in the assessed
  package. Keep the full development project separately.
- The readMe is at most one page in plain text or Markdown and states supported
  behaviour, compile/run commands, important limits, and input format succinctly.
- Build and run a copy extracted into a new directory using the readMe commands
  and the supplied Moodle graphics files. Do not rely on absolute personal paths.
- Source comments attribute copied/generated code and non-trivial borrowed
  algorithms/data structures as required. Attribution alone does not replace
  the coursework's own-effort requirement for an assessed submission.

Sources: coursework sheet, previously pasted Moodle FAQ/instructor replies, and
the lecture references mapped in `IMPLEMENTATION_GUIDE.md`.
