#!/usr/bin/env python3
"""Exercise real DrawApp command output without requiring a Java display."""

import argparse
import os
from pathlib import Path
import re
import shlex
import subprocess
import tempfile


PROJECT = Path(__file__).resolve().parents[1]
ARITY = {
    "LW": 1, "DL": 4, "DR": 4, "DT": 5, "FR": 4, "FT": 5,
    "DO": 4, "FO": 4, "DA": 6, "FA": 6, "RG": 3,
    "CL": 0, "SW": 2, "SL": 1, "FG": 0, "BG": 0, "SZ": 1,
}
TEXT_ARITY = {"DS": 2, "SR": 3, "DI": 2, "DU": 2, "MS": 0}
COLOURS = {
    "black", "blue", "cyan", "darkgray", "gray", "green", "lightgray",
    "magenta", "orange", "pink", "red", "white", "yellow",
}
SUMMARY = re.compile(
    r"(?P<status>[^\n]+): (?P<moves>\d+) moves, (?P<turns>\d+) turns, "
    r"(?P<actions>\d+) actions; (?P<iterations>\d+) iterations, depth (?P<depth>\d+)\."
)


def check(condition, message):
    if not condition:
        raise AssertionError(message)


def protocol(output):
    """Parse the grammar emitted by the supplied, unmodified graphics.c."""
    check(len(output.encode()) < 20_000_000, "Command record exceeds the test's 20 MB bound")
    commands = []
    window = None
    for line_number, line in enumerate(output.splitlines(), 1):
        check(bool(line), f"Empty/non-protocol stdout line {line_number}")
        tokens = line.split()
        op = tokens[0]
        label = None
        if op in TEXT_ARITY:
            check(" @" in line, f"Missing text delimiter on line {line_number}")
            head, label = line.split(" @", 1)
            raw = head.split()[1:]
            check(len(raw) == TEXT_ARITY[op], f"Wrong text arity: {line}")
        elif op == "SC":
            check(len(tokens) == 2 and tokens[1] in COLOURS, f"Invalid colour: {line}")
            commands.append((op, [], tokens[1]))
            continue
        elif op in {"DP", "FP"}:
            raw = tokens[1:]
        else:
            check(op in ARITY, f"Unknown command/non-protocol stdout: {line}")
            raw = tokens[1:]
            check(len(raw) == ARITY[op], f"Wrong command arity: {line}")
        check(all(re.fullmatch(r"-?\d+", token) for token in raw), f"Non-integer argument: {line}")
        numbers = [int(token) for token in raw]
        if op in {"DP", "FP"}:
            check(numbers and numbers[0] >= 3 and len(numbers) == 1 + 2 * numbers[0],
                  f"Invalid polygon: {line}")
        if op == "SW":
            window = tuple(numbers)
            check(0 < window[0] <= 1920 and 0 < window[1] <= 1440,
                  f"Unexpected window extent: {window}")
        if op == "RG":
            check(all(0 <= value <= 255 for value in numbers), f"Invalid RGB colour: {line}")
        if op in {"SL", "SZ", "LW"}:
            check(numbers[0] >= 0, f"Invalid nonnegative argument: {line}")
        if op in {"DL", "DR", "FR", "DO", "FO", "DP", "FP", "DS"}:
            check(window is not None, "Drawing issued before the window size")
            width, height = window
            if op in {"DR", "FR", "DO", "FO"}:
                x, y, w, h = numbers
                check(w >= 0 and h >= 0 and 0 <= x <= x + w <= width and
                      0 <= y <= y + h <= height, f"Shape outside the window: {line}")
            else:
                points = numbers[1:] if op in {"DP", "FP"} else numbers
                check(all(0 <= x <= width and 0 <= y <= height
                          for x, y in zip(points[::2], points[1::2])),
                      f"Drawing origin/point outside the window: {line}")
        commands.append((op, numbers, label))
    check(commands, "No drawing commands emitted")
    return commands


def check_layers(commands):
    check(sum(op == "SW" for op, _, _ in commands) == 1, "Window should be sized once")
    check(sum(op == "BG" for op, _, _ in commands) == 1, "Static maze background redrawn")
    layer = None
    clears = {"BG": 0, "FG": 0}
    foreground_started = False
    polygons = 0
    for op, _, _ in commands:
        if op in {"BG", "FG"}:
            check(op != "BG" or not foreground_started, "Background changed after animation began")
            layer = op
            foreground_started |= op == "FG"
        elif op == "CL":
            check(layer is not None, "Clearing without an explicit layer")
            clears[layer] += 1
        elif op == "FP":
            check(layer == "FG", "Robot polygon must be drawn on the foreground")
            polygons += 1
    check(clears["BG"] == 1 and clears["FG"] >= 1, f"Unexpected initial layer clears: {clears}")
    check(polygons > 0, "No foreground robot polygon")


def dashboard_number(commands, label):
    texts = [(values, text) for op, values, text in commands if op == "DS"]
    labels = [values for values, text in texts if text.strip().upper() == label]
    check(labels, f"Dashboard missing {label}")
    label_x, label_y = labels[-1]
    values_at_position = {}
    for values, text in texts:
        if re.fullmatch(r"\d[\d,]*", text.strip()):
            values_at_position[tuple(values)] = int(text.replace(",", ""))
    candidates = [(y - label_y, value) for (x, y), value in values_at_position.items()
                  if abs(x - label_x) <= 4 and 0 < y - label_y <= 75]
    check(candidates, f"No numeric value below {label}")
    return min(candidates)[1]


def check_dashboard(commands, summary, expected_open, solved):
    labels = [text for op, _, text in commands if op == "DS"]
    joined = "\n".join(labels).upper()
    for label in ["MOVES", "TURNS", "EXPLORATION", "SEARCH", "FRONT SENSOR",
                  "RUN BUDGET", "PLAYBACK TIME"]:
        check(label in joined, f"Missing essential dashboard label: {label}")
    if "ITERATIVE DEEPENING" in joined:
        check("PASS" in joined and "DEPTH" in joined, "IDDFS pass/depth missing")
    else:
        check("PRIORITY" in joined and "RIGHT > FORWARD > LEFT > BACK" in joined,
              "Wall-following decision rule missing")
    for key in ["Wall", "Unvisited", "1 visit", "2-3 visits", "4+ visits",
                "Entrance", "Exit", "Robot", "Clear", "Blocked"]:
        check(key in labels, f"Cell key missing {key}")
    check(labels.count("MOVES") == 1, "Static dashboard labels redrawn during playback")
    check(any(op == "MS" and "CPU (search + drawing generation)" in label
              for op, _, label in commands), "Final CPU summary missing")
    check(("EXIT FOUND" if solved else "SEARCH STOPPED") in joined, "Incorrect final status")
    check(dashboard_number(commands, "MOVES") == summary["moves"], "Dashboard moves differ from robot")
    check(dashboard_number(commands, "TURNS") == summary["turns"], "Dashboard turns differ from robot")
    coverage = []
    for text in labels:
        match = re.search(r"(\d+)\s*/\s*(\d+)", text)
        if match:
            visited, total = map(int, match.groups())
            if total == expected_open:
                coverage.append(visited)
    check(coverage and coverage[0] == 1, "Exploration should begin with the entrance visited")
    check(all(1 <= value <= expected_open for value in coverage), "Exploration count outside grid bounds")
    check(coverage == sorted(coverage), "Unique visit count decreased")
    check(coverage[-1] <= summary["moves"] + 1, "Turns appear to count as new visited squares")


def compile_program(destination, sources, drawapp_dir):
    compiler = shlex.split(os.environ.get("CC", "clang"))
    subprocess.run(compiler + ["-std=c17", "-Wall", "-Wextra", "-Wpedantic", "-g",
                              "-DMAZE_DRAWAPP", "-I", str(PROJECT), "-I", str(drawapp_dir)] +
                   [str(source) for source in sources] + [str(drawapp_dir / "graphics.c"),
                                                         "-o", str(destination)], check=True)


def run_case(executable, arguments, delay, allowed_results):
    common = [str(executable), *arguments]
    preview = subprocess.run(common + ["--preview"], cwd=PROJECT, capture_output=True, text=True,
                             check=True, timeout=20)
    rows = preview.stdout.splitlines()
    check(rows and all(len(row) == len(rows[0]) for row in rows), "Invalid independent preview")
    expected_open = sum(char != "#" for row in rows for char in row)
    result = subprocess.run(common + ["--commands", "--delay", str(delay)], cwd=PROJECT,
                            capture_output=True, text=True, timeout=20)
    check(result.returncode in allowed_results, f"Unexpected run status: {result.stderr}")
    commands = protocol(result.stdout)
    check_layers(commands)
    match = SUMMARY.search(result.stderr)
    check(match is not None, f"Missing stderr statistics: {result.stderr}")
    summary = {key: int(value) for key, value in match.groupdict().items() if key != "status"}
    check(summary["moves"] + summary["turns"] == summary["actions"], "Action accounting mismatch")
    check_dashboard(commands, summary, expected_open, result.returncode == 0)
    sleeps = [values[0] for op, values, _ in commands if op == "SL"]
    check(sum(sleeps) == delay * summary["actions"], "Playback sleep does not match action timing")
    check(all(0 < portion <= delay for portion in sleeps), "Invalid animation sleep portion")
    if summary["actions"] >= 100:
        check(len(commands) < 80 * summary["actions"] + 10000,
              "Excess drawing commands per action")
    print(f"PASS DrawApp {arguments}: {summary['actions']} actions, {len(commands)} commands")


def probe_renderer(directory, drawapp_dir):
    source = directory / "renderer_probe.c"
    source.write_text(r'''
#include <assert.h>
#include <stdio.h>
#include "maze.h"
#include "render.h"
#include "robot_internal.h"

int main(void)
{
    Maze maze = {0};
    assert(mazeCreate(&maze, 5, 5));
    maze.entranceX = 0;
    maze.entranceY = 2;
    maze.startDirection = EAST;
    for (int x = 0; x < 5; x++) mazeSetTile(&maze, x, 2, TILE_EMPTY);
    mazeSetTile(&maze, 4, 2, TILE_EXIT);
    SearchStats stats = {0};
    RenderSettings settings = {.mode = RENDER_DRAWAPP, .delay = 17,
                               .solver = SOLVER_DEEPENING, .stats = &stats};
    Renderer renderer;
    assert(renderInit(&renderer, &maze, &settings));
    Robot *robot = robotCreate(&maze, renderRobot, &renderer);
    assert(robot != 0);
    fprintf(stderr, "PROBE_CELL %d %d %d\n", renderer.gridX + renderer.cellSize,
            renderer.gridY + 2 * renderer.cellSize, renderer.cellSize);
    assert(renderer.visitedCount == 1 && renderer.passageCount == 5);
    assert(renderer.visits[10] == 1);
    assert(renderer.moves == 0 && renderer.turns == 0);
    right(robot);
    left(robot);
    assert(renderer.visitedCount == 1 && renderer.turns == 2);
    assert(renderer.visits[10] == 1);
    forward(robot);
    assert(renderer.visitedCount == 2 && renderer.moves == 1);
    right(robot); right(robot); forward(robot);
    assert(renderer.visitedCount == 2 && renderer.moves == 2);
    right(robot); right(robot); forward(robot);
    assert(renderer.visitedCount == 2 && renderer.moves == 3);
    assert(renderer.visits[11] == 2 && renderer.visits[10] == 2);
    for (int i = 0; i < 3; i++)
    {
        right(robot); right(robot); forward(robot);
        right(robot); right(robot); forward(robot);
        assert(renderer.visits[11] == (i == 0 ? 3 : 4));
    }
    for (int i = 0; i < 3; i++) forward(robot);
    assert(atMarker(robot));
    assert(renderer.visitedCount == 5 && renderer.moves == 12 && renderer.turns == 18);
    assert(renderer.moves == robotMoves(robot) && renderer.turns == robotTurns(robot));
    stats.actions = 30;
    stats.depth = 4;
    stats.iterations = 1;
    renderFinish(&renderer, 1);
    robotDestroy(robot);
    renderDestroy(&renderer);
    mazeDestroy(&maze);
    return 0;
}
''')
    executable = directory / "renderer_probe"
    compile_program(executable, [source, *(PROJECT / name for name in
                    ["maze.c", "robot.c", "solver.c", "render.c"])], drawapp_dir)
    result = subprocess.run([str(executable)], capture_output=True, text=True, check=True, timeout=20)
    commands = protocol(result.stdout)
    check_layers(commands)
    check_dashboard(commands, {"moves": 12, "turns": 18}, 5, True)
    check(sum(values[0] for op, values, _ in commands if op == "SL") == 30 * 17,
          "Initial snapshot or revisit has incorrect playback timing")
    match = re.search(r"PROBE_CELL (\d+) (\d+) (\d+)", result.stderr)
    check(match is not None, "Missing probe cell geometry")
    px, py, cell = map(int, match.groups())
    rgb = None
    fills = []
    for op, values, _ in commands:
        if op == "RG": rgb = tuple(values)
        if op == "FR" and values == [px, py, cell, cell]: fills.append(rgb)
    check({(255, 255, 255), (222, 244, 235), (231, 223, 248), (250, 215, 220)} <= set(fills),
          "A visit band was not actually painted on the revisited passage cell")
    check(fills[-1] == (250, 215, 220), "Frequently revisited cell lost its colour")
    print("PASS renderer observer: initial visit, turns, revisits, final counters, visit colours/key, smooth-frame timing")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", nargs="?", type=Path)
    parser.add_argument("--drawapp-dir", type=Path,
                        default=Path(os.environ["DRAWAPP_DIR"]) if "DRAWAPP_DIR" in os.environ else None,
                        help="Directory containing the genuine Moodle graphics.h and graphics.c")
    arguments = parser.parse_args()
    drawapp_dir = arguments.drawapp_dir
    if drawapp_dir is None and all((PROJECT / name).is_file()
                                  for name in ["graphics.h", "graphics.c"]):
        drawapp_dir = PROJECT
    if drawapp_dir is None or not all((drawapp_dir / name).is_file()
                                       for name in ["graphics.h", "graphics.c"]):
        parser.error("Pass --drawapp-dir with the supplied, unmodified Moodle graphics files")
    drawapp_dir = drawapp_dir.resolve()
    with tempfile.TemporaryDirectory(prefix="maze-drawapp-tests-") as temporary:
        directory = Path(temporary)
        executable = arguments.executable.resolve() if arguments.executable else directory / "maze-draw"
        if arguments.executable is None:
            compile_program(executable, [PROJECT / name for name in
                            ["main.c", "config.c", "maze.c", "generator.c", "robot.c",
                             "solver.c", "render.c"]], drawapp_dir)
        run_case(executable, ["--generate", "5", "5", "--seed", "42"], 0, {0})
        run_case(executable, ["-f", "mazes/looped.txt", "--solver", "iddfs"], 0, {0})
        run_case(executable, ["--generate", "35", "35", "--seed", "1", "--solver", "wall",
                              "--budget", "500"], 0, {0, 2})
        run_case(executable, ["--generate", "5", "5", "--seed", "42", "--solver", "wall"], 17, {0})
        run_case(executable, ["--generate", "5", "5", "--budget", "3"], 0, {2})
        for width, height in [(101, 101), (101, 5), (5, 101)]:
            run_case(executable, ["--generate", str(width), str(height), "--seed", "42",
                                  "--budget", "25"], 0, {0, 2})
        probe_renderer(directory, drawapp_dir)
    print("DrawApp protocol/dashboard checks passed with the genuine graphics implementation.")


if __name__ == "__main__":
    main()
