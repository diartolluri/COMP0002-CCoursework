#!/usr/bin/env python3
"""Keep the search translation unit limited to the five robot operations."""

from pathlib import Path
import os
import re
import shlex
import subprocess
import tempfile

project = Path(__file__).resolve().parents[1]
source = project / "solver.c"
text = source.read_text()
includes = re.findall(r'^\s*#\s*include\s*[<"]([^>"]+)', text, re.MULTILINE)
assert set(includes) <= {"solver.h", "limits.h", "stdlib.h"}, includes
header = (project / "solver.h").read_text()
assert '"robot.h"' in header
assert not re.search(r'\b(?:Maze|RobotView|mazeTileAt|robotView|robotMoves|robotTurns)\b', text + header)
robot_header = (project / "robot.h").read_text()
assert re.search(r'typedef\s+struct\s+Robot\s+Robot\s*;', robot_header)
assert not re.search(r'struct\s+Robot\s*\{', robot_header)

with tempfile.TemporaryDirectory(prefix="maze-interface-") as directory:
    obj = Path(directory) / "solver.o"
    compiler = shlex.split(os.environ.get("CC", "clang"))
    subprocess.run(compiler + ["-std=c17", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
                              "-I", str(project), "-c", str(source), "-o", str(obj)], check=True)
    symbols = subprocess.run(["nm", "-u", str(obj)], capture_output=True, text=True, check=True).stdout
    undefined = set()
    for line in symbols.splitlines():
        if line.strip():
            undefined.add(line.split()[-1].lstrip("_"))
    allowed = {"forward", "left", "right", "atMarker", "canMoveForward", "malloc", "free"}
    assert undefined <= allowed, f"Unexpected solver dependencies: {sorted(undefined - allowed)}"
    assert {"forward", "left", "right", "atMarker", "canMoveForward"} <= undefined

print("Solver interface passed: opaque robot; only five robot operations and allocation dependencies.")
