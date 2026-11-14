# Design and evidence for the reference implementation

This is a learning/model solution, developed with OpenAI Codex in November 2026.
The coursework's own-work and attribution rules remain relevant to any later
assessed use. No mark is predicted: the rubric rewards appropriate design, clarity,
working behaviour, novelty, and quality, rather than a count of language features.

## State and component boundaries

`Maze` owns a contiguous heap allocation plus dimensions and entrance metadata.
`mazeTileAt` converts signed coordinates to an index only after checking bounds.
Outside the grid is a wall. Loading/generation build a temporary value, validate
it, then replace the old maze; failures preserve the caller's previous object.
Callers initialise a Maze to zero and destroy it once. File loading accepts spaces
and dots, CRLF or LF, and a final row without a newline; it rejects malformed
rows, symbols, boundary openings and unreachable exits.

The robot implementation owns its mutable position, orientation and counters.
Its public header exposes an incomplete `Robot` type and only the five coursework
operations. World setup and rendering access are in `robot_internal.h`, which the
solver must not include. The solver is compiled without maze/header dependencies
and receives no coordinates, dimensions, distances, adjacency, or visited map.
The validator's full-world BFS is separate: none of its route/visited data enters
search. Text and graphical rendering read state through an observer callback.

Options are parsed once. A small supplied-field mask records explicit generation
and timing values; preset defaults fill only the remaining fields afterwards.
Thus argument order does not change explicit overrides, without a second parser.
The launcher skips option values when selecting its output pipeline, so a file
literally named `--headless` is not mistaken for a mode switch.

## Search through the five robot operations

The default right-wall follower is a face-boundary traversal of the planar
passage graph. The boundary entrance with an inward-facing start selects the
exterior face, which also contains a reachable boundary exit. Thus passage loops
and wall islands do not invalidate it under these exact coursework constraints.
It is not a general arbitrary-interior-start/goal solver. A coarse action bound
is below 20 times width times height; the default two-million budget comfortably
covers the supported dimensions. Tests exhaust every reachable 5x5 boundary case.

Wall-following reference: https://yozh.readthedocs.io/en/latest/projects/maze1.html .


### Optional iterative-deepening comparison

The optional solver repeats depth-limited DFS with limits 1, 2, 3, ... . A frame
records only a remaining move allowance, the next relative direction to try, and
how to restore its parent's heading. Frames form an owned linked stack on the
heap, so search depth does not consume the C call stack. This applies linked
structures, pointer ownership, and the semantics of recursive backtracking.

A failed branch returns to its entry square with its entry direction restored.
A child physically turns around, moves back, turns around again, and undoes the
relative turn that selected it. No direct coordinate updates are available in
this module. The root tries all four directions; descendants omit immediate
reversal because a shortest route cannot contain an immediate out-and-back move.
Every forward step is followed by a marker check before further action.

At a finite depth, there are finitely many branches, including in a cyclic maze.
If an exit is reachable, some finite shortest path has no immediate reversal.
That path will therefore be considered at its depth limit. This establishes
completeness in the absence of resource exhaustion. Repeated-state detection is
intentionally absent because the robot is not given a map.

Time can be exponential in exit distance on cyclic mazes, roughly O(3^d) for
this non-reversing action tree, with O(d) active search memory. On tree-like mazes
there are no non-backtracking cycles, which makes repeated passes much cheaper.
The final successful depth is shortest forward distance, not the total animated
journey: earlier passes and backtracking may involve many moves. We do not call
the animation a shortest route.

A default action budget prevents accidentally unbounded demonstrations. Every
issued turn/move consumes it. On exhaustion or allocation failure, frames are
freed and the search reports a resource error; it does not claim the world is
unsolvable. An aborted search may stop partway through a restoration sequence,
leaving a valid position/orientation without restoring the initial pose. A cyclic
unreachable world cannot generally be certified unreachable by this map-free
method; normal inputs are independently checked for reachability first.


Algorithm reference (conceptual, no source code copied): Russell and Norvig,
*Artificial Intelligence: A Modern Approach*, iterative-deepening/depth-limited
search: https://aima.cs.berkeley.edu/4th-ed/pdfs/newchap03.pdf . The implementation
adapts the search to a physically moving, sensor-only robot.

## Generation and validation

Generation uses randomised depth-first carving of logical cells separated by
walls. Marking a new cell before descending prevents revisiting it during carving
and produces a connected tree of passages. The supported odd dimensions cap the
number of logical cells at 2,500. Additional eligible internal walls can be opened
according to `--loops`, creating cycles while preserving connectivity and the
outer wall. A local, explicitly 32-bit unsigned generator makes a seed reproducible
without process-global random state. This is for maze variety, not cryptography.
The LCG parameters are those described by Press et al., *Numerical Recipes in C*,
second edition, section 7.1. The BFS validator follows the standard FIFO traversal
described in Russell/Norvig, *AIMA*, chapter 3; it returns no route to the solver.

Direction candidates are represented by unsigned bit masks. A candidate is tested
for nonzero, not equality to 1. Entrances/exits are added on the boundary, and
an independent BFS checks reachability. These full-world operations belong to
construction/validation, not the robot's knowledge.

Conceptual generation reference: Jamis Buck, *Maze Generation: Recursive
Backtracking*, https://weblog.jamisbuck.org/2010/12/27/maze-generation-recursive-backtracking .
No Ruby code or other library is incorporated.

## Applying the lecture material

| Material | Concrete use |
| --- | --- |
| 1: methodical development | Working increments, independent checks, clear stated limitations. |
| 2 and 5: compilation, linking, memory | Separate objects, guarded headers, warnings/debug builds, standard C17, sanitizers. |
| 3 and 11: drawing, pipelines and I/O | Genuine DrawApp integration, separate foreground/background, stderr diagnostics, ordinary stdio file input. |
| 4 and 6: abstraction, control and operators | Only five robot operations, explicit action sequencing, bounded loops and direction arithmetic. |
| 7 and 8: arrays and strings | Contiguous grid, safe indexing, array traversal, line-format validation and checked string options. |
| 9: functions and recursion | Cohesive functions, backtracking search semantics, recursive generator. |
| 10 and 15: scope and interfaces | Private static helpers, opaque robot, public/internal header separation, renderer independence. |
| 12 and 13: pointers, const and structs | Owned/borrowed pointers, views, immutable maze access during solving, designated initialisation. |
| 14: dynamic structures and callbacks | Heap-owned grid, linked search frames, observer and action function pointers, failure cleanup. |
| 17: bit operations | Unsigned generator direction masks and explicit fixed-width PRNG arithmetic. |

Features such as generic void-pointer containers, sorted linked lists, Unicode
maze parsing, arbitrary polygon turtles and C23-only syntax were considered and
omitted because they would not improve this program. The observer's context uses
`void *` at an appropriate callback boundary; application data otherwise remains
typed. Signed coordinates avoid unsigned north/west underflow; buffer sizes use
`size_t`. Short interpolated drawing frames make movement easier to follow;
the robot's actual position still changes by exactly one grid square.

## Evidence and limits

`make test` checks robot/solver/generator contracts and executable/file behaviour.
`make sanitize` runs meaningful cases under address and undefined-behaviour
sanitizers. The test-only shortest-distance oracle uses an independent BFS and
never supplies information to the robot. The architecture check guards the solver
interface. Test logs and actual counts should be recorded after checks finish.

The renderer now compiles against the genuine supplied DrawApp 5.0 files, kept
byte-identical to the course originals. Its background is static; foreground
updates contain the robot, visualization-only visitation shading and dashboard.
Live statistics show moves, turns, visited cells, coverage, search pass/depth,
actions/budget and simulated playback time. They never enter the solver.

The real Java viewer was inspected on a generated 35x35 maze and a cyclic IDDFS
example. The canvas was compacted after the first visual check revealed clipping
from DrawApp's own playback/message controls. Pause/Play, Reset, live statistics
and the completed view were checked. A screenshot is in docs/drawapp-preview.png.
`./run.sh` now launches the graphical experience by default; explicit headless
and preview modes remain available for testing. See VALIDATION.md for evidence.

`make package` creates an allowlisted source/data/readMe ZIP and excludes the
DrawApp distribution, development notes/tests/scripts, generated data and binaries.

## Presentation improvements

Static dashboard cards/labels stay on the background; foreground values are cached
and only refreshed when changed. Fast runs throttle numeric updates, while every
robot action retains its animation and explanation. A front-cell preview and
sensor-only decision descriptions explain turns without changing solver access.
The renderer-owned visitation data supplies revisits and new-cell rate only to
presentation. Final producer CPU time includes C search and command generation,
not Java playback. Presets offer reproducible small, large and cyclic demos.

Each renderer cell holds an arrival count saturated at four. The initial snapshot
counts once; later snapshots count only coordinate changes, never turns. White,
mint, lavender and rose represent zero, one, two–three and four-plus arrivals.
Entrance blue and exit gold override those fills. A two-row key stays below the
maze, with numeric visit labels on larger tiles and dots on smaller tiles. This
array supplies only presentation and unique-cell statistics, never search input.
