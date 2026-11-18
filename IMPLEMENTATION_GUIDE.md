# Coursework implementation guide

Implementation update: the model solution now includes the modules and checks
described in DESIGN.md. This original study guide records the earlier planning
phase; README.md documents current commands. The robot is now opaque, the maze
is dynamically sized, and the general search uses explicit linked heap frames.
The genuine DrawApp 5.0 distribution is now integrated and visually checked.
Wall-following is now the default solver; IDDFS is an optional comparison.
The default launcher opens the graphical dashboard; see the current running
guide and VALIDATION.md rather than the original planning-stage commands below.

Reviewed 14 November 2026. This is a development reference for the learning/model
solution discussed in this conversation, separate from the short program readMe.

## 1. Material reviewed and how to use it

The review covers the coursework sheet and all 16 supplied lecture decks:
Slides1-15 (with `Slides4.initial.pdf` as lecture 4), plus Slides17. This is 439 PDF
pages in total. There is no Slides16, as confirmed in the conversation. Text was
reviewed throughout; selected maze, drawing, linked-list, and bitwise diagrams
were also checked visually. Page references below mean PDF page positions;
printed slide numbers in the pointers deck sometimes differ.

Treat the coursework sheet as the project specification. Lecture examples explain
concepts and sometimes deliberately show incomplete or incorrect approaches.
The previously pasted Moodle replies clarify standard file I/O, animation timing,
and the restriction on the robot's knowledge. Proposed architecture and maze
algorithms below are design recommendations, not extra requirements quoted from
the sheet or complete solutions supplied by the slides.

Use the full body of teaching to make sound choices. There is no requirement to
insert every operator, data structure, or library function into the program.
Readability and a straightforward working design are explicit marking concerns.

## 2. Coursework requirements

### Program and maze

- Write C that compiles with gcc or clang.
- Display and animate a robot travelling from the maze entrance to the exit marker
  using the supplied DrawApp graphics code.
- Draw the maze and exit once on the background layer. Update the robot on the
  foreground layer.
- Represent a rectangular grid of equal-sized squares. Each square is empty,
  a wall, or the exit marker.
- Coordinates are `(x, y)`: column from the left, row from the top. `(0, 0)` is the
  top-left square.
- The outer wall is one square thick, with exactly two single-square openings:
  one entrance and one exit. Put the marker on the exit square.
- Start the robot on the entrance square, facing into the maze.
- Ensure at least one horizontal/vertical path connects entrance and exit.
- Treat every position outside the grid as a wall.
- Support the dead ends and loops relevant to the claimed search capability.
  A general solver must not become permanently trapped in a loop.
- Make the whole maze fit in the drawing window, resizing the window if needed.

### Robot contract

| Operation | Required behaviour |
| --- | --- |
| `forward` | Move one square in the facing direction if it is clear; otherwise remain still. |
| `left` | Turn 90 degrees anticlockwise without changing position. |
| `right` | Turn 90 degrees clockwise without changing position. |
| `atMarker` | Return an integer truth value indicating whether the robot is on the exit. |
| `canMoveForward` | Return an integer truth value indicating whether the next square is clear. |

The sheet permits parameters and, where useful, return values for the movement
functions. The grid may exist in the world implementation for rendering,
collision detection, validation, and generation. This project's stricter solver
interface makes movement decisions through the robot operations. It does not
inspect the grid, consult a map built by generation, teleport the robot, or
directly alter its coordinates.
The sheet does not unambiguously forbid all higher-level knowledge or memory.
Following the pasted instructor clarification, this design does not give the solver
a discovered-map or visited-cell matrix either.

Counters, local control variables, and recursion are useful for organising a
sequence of calls; they must not become a hidden source of maze knowledge.

### Five stages

| Stage | Deliverable | Evidence that it works |
| --- | --- | --- |
| 1 | Display a designed maze. | The complete grid and exit appear correctly in DrawApp. |
| 2 | Animate the robot along a predetermined route. | Visible moves and turns, collision checks, correct exit stop. |
| 3 | Implement a basic maze search. | Decisions come from sensors; demonstrate the maze classes it solves. |
| 4 | Generate mazes with one entrance and one exit. | Generated grids obey the rules and contain a route between openings. |
| 5 | Solve every maze the generator can produce. | State the algorithm's assumptions, explain why they match generation, and test many cases. |

The sheet encourages incremental progress. Basic stages with reasonable code can
earn a pass; completing every stage is not a guarantee of a particular mark.

### Libraries, runtime, and portability

- Use standard C libraries plus the Moodle `graphics.h` and `graphics.c` only.
- Do not modify either supplied graphics file or add third-party code libraries.
- Do not use platform-specific headers such as `unistd.h` or `windows.h`.
- The sheet does not require C23. C17 is a reasonable explicit baseline for the
  existing scaffold. Compile examples under the chosen standard before reusing
  them; some lectures show C23-only syntax.
- Use DrawApp's `sleep(milliseconds)` for animation timing, following the Moodle
  guidance. Do not replace it with an operating-system sleep API.
- In graphics mode, reserve stdout for drawing commands. Send diagnostics to
  stderr. Keep the existing text preview as a separate mode.
- Command-line configuration and file input are permitted. Standard `stdio.h`
  provides `FILE`, `fopen`, `fgets`/`fgetc`, and `fclose`; no POSIX option parser
  or low-level file access is necessary.
- Check the real DrawApp header before implementing foreground/background,
  clearing, polygon, and timing calls. The slides do not reproduce its full API.

### Code quality and documentation

Prefer short cohesive functions, clear names, consistent layout, minimal
duplication, appropriate types, and explicit error handling. Comments should add
design rationale or attribution rather than narrate obvious statements. Use
multiple source/header files where responsibilities justify them.

For assessed submission, the sheet requires the student's own work, references
in source comments for copied code and non-trivial borrowed algorithms/data
structures (including AI code), and writing as much code independently as
possible. Lecture 2 also says that where no AI category is stated, AI is
assistive and the work must remain substantially the student's own. This guide
does not reclassify those rules because the current project is a model solution.

If preparing an assessed package, the stated deadline is 18 November 2026 at
16:00 UK time, subject to individual adjustments. Name the ZIP
`COMP0002CW1.zip`; include source, required data/images, and a plain-text or
Markdown readMe of at most one page. Exclude compiled artifacts, DrawApp files
(`graphics.h`, `graphics.c`, and the JAR), names/student numbers, development
notes, and intermediate versions. The readMe should describe the program and
give accurate compile/run commands.

## 3. Every lecture mapped to implementation

| Deck and useful pages | Teaching considered | Application to this project |
| --- | --- | --- |
| Lecture 1, pp. 2-14 | Practice, methodical work, attention to detail, understanding AI output, avoiding unnecessary complexity. | Build small working increments, understand and explain each function, keep backups, and review generated code. This lecture supplies methodology rather than a maze algorithm. |
| Lecture 2, pp. 9-11, 15-25, 27-30 | C types and responsibility, `main`, `argc`/`argv`, compile/run cycle, compiler phases, warnings, debugging, comments, C-standard selection. | Parse options, return useful exit codes, compile with warnings and debug information, distinguish compile/link/runtime failures, and use sanitizers while developing. C history and cross-compilation are background, not application features. |
| Lecture 3, pp. 3-6, 8-19 | Graphics primitives, includes, compile/link graphics code, pipes, paths, coordinates, drawing order, decomposition. | Draw walls/marker/robot, calculate pixel positions from grid coordinates, use relative asset paths, pipe drawing commands into DrawApp, and verify required Java/DrawApp setup. |
| Lecture 4, pp. 3-9, 13-34, 40-43 | Sequence, selection, iteration, robot state, sensors, abstraction, counterexamples, simplifying a problem. | Implement the five robot functions and build sensor-driven control loops. Check for the marker after movement. Do not mistake the empty-arena examples for universal maze solvers. |
| Lecture 5, pp. 9-13, 17-29 | Memory layout, stack/heap/static data, compilation, object files, linking, libraries, loading. | Keep memory lifetimes valid, compile modules separately and link once, understand dependency failures, and use `sizeof` rather than guessed byte counts. Buses, GPU memory, DMA, and embedded device registers are background; the coursework forbids platform-specific hardware additions. |
| Lecture 6, pp. 5-30, 33-48 | Initialisation, arithmetic, conversions, integer division, precedence versus evaluation order, comparisons, logical operators, short-circuiting, loops, off-by-one mistakes. | Initialise all state, implement direction arithmetic, iterate through cells, validate ranges before indexing, and sequence robot actions explicitly. Keep grid coordinates integral; floating point is only useful for optional rendering calculations. |
| Lecture 7, pp. 3-15, 18-38 | Arrays, contiguous storage, lengths, initialisation, bounds, `size_t`, `sizeof`, 2D arrays, nested loops, array parameters, lifetime pitfalls. | Store the maze as rows/columns, iterate with known dimensions, check bounds before access, pass dimensions with buffers, and never return pointers to local arrays. VLAs are considered but not needed for a simple portable baseline. |
| Lecture 8, pp. 4-18, 20-22 | Null-terminated strings, character arrays, `strlen`, `strcmp`, copying, bounded input, `fgets`, array decay, Unicode byte counts. | Compare options using `strcmp`; define a simple ASCII maze format; preserve spaces; reserve room for `\0`; handle line lengths/newlines explicitly. Maze files do not need Unicode processing or unsafe string-copy exercises. |
| Lecture 9, pp. 2-16, 19-26 | Function decomposition, parameters versus arguments, returns, prototypes, recursion and its limits, `void`. | Keep generation, sensing, drawing, and solving separate. Use recursion where it simplifies exploration, with a real base case and depth/resource policy. A specific backtracking maze algorithm is a design adaptation of these ideas. |
| Lecture 10, pp. 2-7, 9-22 | Scope versus lifetime, shadowing, file/block scope, `static`, `extern`, internal/external linkage, localisation. | Prefer locals, pass owned state explicitly, make private helpers `static`, declare public functions in headers, avoid shared mutable globals, and avoid confusing variable shadowing. `extern` knowledge informs the choice not to share world state globally. |
| Lecture 11, pp. 2-17 | Streams, stdin/stdout/stderr, formatting, buffering, character input, EOF, short-circuiting, redirection and pipes. | Send only drawing instructions to stdout in graphics mode; send errors/status to stderr. Use line/character-based maze input with checked return values. Store `fgetc`/`getchar` results in `int` until EOF is checked. Interactive prompts do not suit the normal DrawApp pipeline. |
| Lecture 12, pp. 7-21, 29-34 | Addresses, indirection, pointer parameters, NULL/dangling pointers, array decay, pointer arithmetic, `const`, generic pointers, copying. | Pass `Robot *` to state-changing operations, use read-only views where appropriate, store buffer sizes explicitly, and maintain pointer validity. A `Tile **` is not interchangeable with a rectangular `Tile[rows][cols]`. Prefer typed pointers over unnecessary `void *`. |
| Lecture 13, pp. 2-10 | Structs, field access, designated initialisation, struct copies, embedded arrays versus pointer members, typedef, struct pointers. | Group position/direction into `Robot`, and dimensions/storage into `Maze`. Distinguish the current safe by-value maze copy with an embedded array from a future shallow copy of a heap-owning struct. Use `.` for values and `->` for pointers. |
| Lecture 14, pp. 2-7, 8-23, 24-26 | Heap allocation, `malloc`/`calloc`/`free`, NULL failure, ownership, linked lists, insertion/deletion, double pointers, `void *`, function pointers/callbacks. | Dynamic maze dimensions can justify a heap-owned cell buffer. Free each owned allocation exactly once on success and failure. A linked stack could support generator backtracking; a small typed callback could notify rendering. Neither a generic list library, sorted insertion, `qsort`, nor `bsearch` is necessary just to demonstrate the lecture. |
| Lecture 15, pp. 3-5, 8-27, 31-35 | Better abstractions, parameterisation, vector drawing, precision and rounding, named constants, headers, include guards, static helpers, information hiding, separate compilation. | Keep pixel conversion and triangle drawing inside the renderer, cell geometry out of the solver, and helper functions private. Use guarded self-contained headers and include each module's own header first. Four fixed orientations need no general turtle library; if smooth geometry is added, retain precision until rounding for pixels. |
| Lecture 17, pp. 2-11 | Bitwise AND/OR/XOR/complement, shifts, masks, unsigned values, conditional expressions. | Optionally represent generator direction candidates or configuration flags with unsigned masks. Test masked values for nonzero, distinguish `&` from `&&`, and keep shift counts in range. Do not use bit packing to smuggle a world map into the solver. |

## 4. Recommended structure and data ownership

Keep the existing files and expand responsibilities only when needed:

| Module | Responsibility | Boundary to preserve |
| --- | --- | --- |
| `main.c` | Parse configuration, create/load/generate a maze, initialise graphics and robot, choose a solver, clean up, return status. | Coordinate components; keep search logic and graphics calculations elsewhere. |
| `maze.h` / `maze.c` | Tile representation, dimensions, safe lookup, construction/destruction, structural validation. | Bounds checks precede every grid read/write. |
| `robot.h` / `robot.c` | Position, direction, collision handling, and the five movement/sensor operations. | Only these movement operations change robot position/direction during solving. |
| `render.h` / `render.c` | Text preview, static maze drawing, robot geometry/redraw, pixel conversion. | Rendering reads state; it does not choose the route or change world geometry. |
| `solver.h` / `solver.c` | A sequence of calls to the five robot operations. | No grid access, world validation queries, coordinate queries, generated route, or visited-cell map. |
| `generator.h` / `generator.c` | Create connected mazes and select valid openings. | It may use temporary visited data; that data must not be handed to the solver. |

If file input grows, move it into `maze_io.c` rather than expanding `main.c`
indefinitely. Do not introduce additional layers without a clear responsibility.

The current scaffold passes a maze pointer to robot functions and exposes the
structs. That is usable if the solver treats those values as handles and does
not inspect their fields. A stronger later boundary can bind the world to the
robot during setup and hide implementation details behind the robot header.
This is an architectural option, not a new robot sensor or a requirement to
rewrite the entire scaffold first.

For runtime dimensions, a single allocated cell buffer is a straightforward
option: row/column access becomes `cells[y * width + x]` after bounds checks.
Store the dimensions alongside the pointer. Use signed coordinates for movement
so north/west edge checks do not accidentally underflow an unsigned value; use
`size_t` for checked allocation sizes and buffer lengths.

Validate dimensions and multiplication before allocating. State who owns each
buffer. Destroy partially initialised objects safely after allocation or parsing
failure. Copying a struct containing an owned pointer does not duplicate its
allocation; avoid two owners freeing the same cells.

## 5. Implementation sequence and acceptance checks

1. **Retain the current working preview.** The existing fixed grid, robot
   operations, and terminal renderer are the baseline. Keep each build runnable.
2. **Complete Stage 1 in DrawApp.** Inspect the supplied graphics header, draw
   one square first, then the complete maze and exit on the background layer.
   Parameterise cell size/margins and ensure the window contains the whole grid.
3. **Complete Stage 2.** Draw a triangle whose orientation follows the robot.
   Animate a known valid route using the five functions. Clear/redraw only the
   foreground as appropriate for the actual API. Delay using DrawApp's sleep
   function. Check for success before issuing further movement.
4. **Complete basic searching.** Implement and explain an initial sensor-only
   wall-following strategy. Test turns, junctions, dead ends, and the exact maze
   assumptions. Merely turning right after hitting a wall is not a general
   wall-following algorithm, and wall-following is not a universal maze solver.
5. **Generate valid mazes.** A proposed first approach is randomised recursive
   backtracking over logical cells: carve to an unvisited neighbour, recurse,
   and backtrack when none remain. Preserve the outer boundary and open one
   entrance/exit connected to passages. This initially produces mazes without
   passage cycles. Seeded randomness is useful for repeating failures in the
   same environment. The specific algorithm and random-number calls are
   implementation choices, not code taught by these slides.
6. **Establish Stage 5 honestly.** Explain why the chosen solver covers the
   generator's output. With the specified outer entrance/exit and inward start,
   right-wall following reaches a reachable boundary exit even with loops. It is
   now the default. Include cyclic and boundary-orientation tests. Arbitrary
   interior starts/goals would require a different guarantee.
7. **Add useful configuration.** File path, maze size, animation delay, and seed
   are reasonable options. Parse them with standard C, validate ranges, and
   report errors clearly. File loading is optional in the sheet; dynamic sizes
   should be added after a fixed version works, not as a prerequisite to it.
8. **Finish with verification and documentation.** Build all modules with
   warnings, exercise meaningful failures, inspect the real animation, and
   keep the readMe accurate. Exclude this long development guide from any
   assessed submission package.

### A possible general solver without a map

One proposed extension is iterative-deepening depth-first exploration of action
paths. First search up to a small move depth, then repeat with increasing depth.
Each recursive call checks the marker, probes choices through turning and
`canMoveForward`, physically moves forward, and physically backtracks after an
unsuccessful branch. On failure it must restore the caller's position and
orientation; on success it stops at the exit immediately.

At any finite depth there are finitely many choices, so a maze loop cannot cause
an infinite recursive branch. In a finite maze with a reachable exit, some finite
path depth reaches it; increasing the bound eventually includes that path. This
reasoning does not require the solver to read coordinates or build a map.

The tradeoff is repeated exploration, potentially exponential work, and recursive
stack depth. This is an option to evaluate on modest mazes, not an assertion that
recursion alone solves loops efficiently. Define resource limits, report resource
exhaustion distinctly, and never report "no route" merely because a testing move
budget expired. Generation and independent validation may inspect the maze;
the solver still may not. The lectures teach recursion, but do not present this
specific algorithm or its proof.

## 6. Technical checks during implementation

| Area | Checks that matter |
| --- | --- |
| Robot movement | Four left/right turns restore orientation; turning preserves position; walls block moves; entrance/exit never allow movement outside the grid; sensors agree with visible motion. |
| Marker handling | Check after every forward move; an exit reached halfway through a corridor is not skipped; no further movement occurs after success. |
| Maze structure | Rectangular rows, supported tile symbols, exactly two valid boundary openings, one exit marker, correct initial facing, and a verified path. Reject malformed input rather than quietly treating unexpected symbols as empty cells. |
| Search | Straight route, left/right bends, dead ends, multiple branches, and cyclic layouts. Explain the algorithm's domain as well as showing examples. |
| Generation | Different dimensions/seeds, reproducible failures in the same environment, correct boundaries and connected exit. Check full structure independently of solver behaviour. |
| Memory | Valid lifetimes, no out-of-bounds access, no unchecked allocation-size overflow, checked NULL allocations, one free per owner, cleanup on parse/generation failure. |
| Input | Missing/unknown options, invalid/overflowing numbers, nonexistent files, overlong or uneven rows, invalid characters, CRLF/newline handling, optional final line without a newline, and read errors. Never use `scanf("%s")` to read space-containing maze rows. |
| Graphics | Background remains intact, foreground leaves no trails, orientation is visible, all cells fit, timings are usable, and stdout contains only DrawApp commands in graphics mode. |
| Build | All source files included, supplied graphics code linked unchanged, headers self-contained and guarded, warnings resolved, and clean build from documented commands. Use address/undefined-behaviour sanitizers during development where available. |

Testing should target these behaviours, not reproduce the implementation line by
line. A successful animation is not a substitute for boundary and input tests;
passing tests on a few generated seeds is not a proof of general solver coverage.

## 7. Lecture example details to handle carefully

- **Bit tests:** lecture 17 p. 10 says to check a masked result against 1. For a
  bit other than bit 0, the value is a power of two. Test `(flags & mask) != 0`.
  Pages 8 and 11 already use truth/nonzero in the conditional expression.
- **Shift diagram:** lecture 17 p. 4 labels `10000000` as `1 << 8`; the eighth
  position shown corresponds to `1u << 7`. Use unsigned literals and valid shift
  counts, not the diagram's mistaken label.
- **Logical versus bitwise:** `&&` and `||` are short-circuit truth operations;
  `&` and `|` manipulate individual bits and do not provide that guarding
  behaviour. Parenthesise mask comparisons.
- **Portability:** do not assume every `int` is four bytes, every pointer is eight
  bytes, or plain `char` is signed. Use the actual types, `sizeof`, and `CHAR_BIT`
  when bit widths matter. Hardware illustrations are not universal C guarantees.
- **Math constants:** lecture 15's expert appendix corrects earlier use of
  `M_PI`, which is not an ISO C constant. If trigonometry is actually needed,
  use a named own constant or another standard approach, and link the standard
  maths library where required. Four cardinal robot orientations need no trig.
- **Representation:** a cast to `int` truncates; it does not round. Keep any
  optional smooth animation calculations separate from exact integer grid state.
- **C standard:** C23-only syntax and library facilities shown in examples are
  not automatically available under `-std=c17`. Keep the chosen baseline
  consistent, including booleans, null pointers, VLA initialisation, and string
  duplication. Prefer ordinary portable equivalents when they are sufficient.
- **Strings:** example input/copy functions are not permission to overflow a
  buffer. Check capacity and termination; prefer clear bounded input over clever
  compact pointer expressions.
- **Arrays versus pointers:** `sizeof` on an array parameter gives a pointer's
  size. Passing a pointer copies that pointer, not the pointed-to data. A struct
  containing an embedded array can be copied by value safely, whereas a struct
  containing owned pointers needs an explicit ownership policy.
- **Recursion:** a base case must actually be reached. A recursive solver with
  no cycle/depth handling is not made correct merely by using recursion.

## 8. Source map

The links below point to the local PDFs reviewed. These are sources, not files to
bundle with a final coursework program. Earlier Moodle clarifications were pasted
directly into the conversation rather than obtained from the PDFs.

- [Coursework sheet](</Users/diartolluri/University Work/COMP0002 - Principles of Programming/C_Coursework.pdf>)
- [Lecture 1: course introduction](</Users/diartolluri/University Work/COMP0002 - Principles of Programming/Slides1.pdf>)
- [Lecture 2: first look at C](</Users/diartolluri/University Work/COMP0002 - Principles of Programming/Slides2.pdf>)
- [Lecture 3: drawing programs](</Users/diartolluri/University Work/COMP0002 - Principles of Programming/Slides3.pdf>)
- [Lecture 4: imperative programming and robot abstraction](</Users/diartolluri/University Work/COMP0002 - Principles of Programming/Slides4.initial.pdf>)
- [Lecture 5: memory and the compiler](</Users/diartolluri/University Work/COMP0002 - Principles of Programming/Slides5.pdf>)
- [Lecture 6: variables, expressions, and loops](</Users/diartolluri/University Work/COMP0002 - Principles of Programming/Slides6.pdf>)
- [Lecture 7: arrays](</Users/diartolluri/University Work/COMP0002 - Principles of Programming/Slides7.pdf>)
- [Lecture 8: strings](</Users/diartolluri/University Work/COMP0002 - Principles of Programming/Slides8.pdf>)
- [Lecture 9: functions](</Users/diartolluri/University Work/COMP0002 - Principles of Programming/Slides9.pdf>)
- [Lecture 10: scope and lifetime](</Users/diartolluri/University Work/COMP0002 - Principles of Programming/Slides10.pdf>)
- [Lecture 11: terminal I/O](</Users/diartolluri/University Work/COMP0002 - Principles of Programming/Slides11.pdf>)
- [Lecture 12: pointers](</Users/diartolluri/University Work/COMP0002 - Principles of Programming/Slides12.pdf>)
- [Lecture 13: structs](</Users/diartolluri/University Work/COMP0002 - Principles of Programming/Slides13.pdf>)
- [Lecture 14: dynamic memory, lists, and callbacks](</Users/diartolluri/University Work/COMP0002 - Principles of Programming/Slides14.pdf>)
- [Lecture 15: drawing abstractions and component design](</Users/diartolluri/University Work/COMP0002 - Principles of Programming/Slides15.pdf>)
- [Lecture 17: bitwise operators](</Users/diartolluri/University Work/COMP0002 - Principles of Programming/Slides17.pdf>)
