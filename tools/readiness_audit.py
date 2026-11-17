"""Record reproducible readiness evidence without changing application sources."""
import argparse
from collections import deque
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output', type=Path, default=ROOT / 'build' / 'readiness-audit')
arguments = parser.parse_args()
output = arguments.output.resolve()
output.mkdir(parents=True, exist_ok=True)
log = output / 'commands.log'
log.write_text('Readiness verification; commands run from the project directory.\n')


def run(command):
    result = subprocess.run(list(map(str, command)), cwd=ROOT, capture_output=True,
                            text=True, timeout=120)
    with log.open('a') as stream:
        stream.write('\n$ ' + shlex.join(list(map(str, command))) + '\n')
        stream.write(result.stdout)
        if result.stderr:
            stream.write('\n[stderr]\n' + result.stderr)
        stream.write(f'\n[exit {result.returncode}]\n')
    if result.returncode:
        raise RuntimeError(f'Command failed; see {log}: {command}')
    return result


def inspect_grid(text):
    rows = text.splitlines()
    height, width = len(rows), len(rows[0])
    assert 5 <= width <= 101 and 5 <= height <= 101
    assert all(len(row) == width and set(row) <= set('#. SE') for row in rows)
    starts = [(x, y) for y, row in enumerate(rows) for x, tile in enumerate(row) if tile == 'S']
    exits = [(x, y) for y, row in enumerate(rows) for x, tile in enumerate(row) if tile == 'E']
    assert len(starts) == len(exits) == 1
    start, end = starts[0], exits[0]
    boundary = {(x, y) for y, row in enumerate(rows) for x, tile in enumerate(row)
                if tile != '#' and (x in (0, width - 1) or y in (0, height - 1))}
    assert boundary == {start, end}
    assert all(rows[y][x] == '#' for x, y in [(0, 0), (width - 1, 0),
                                            (0, height - 1), (width - 1, height - 1)])

    def neighbours(x, y):
        return [(nx, ny) for nx, ny in [(x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)]
                if 0 <= nx < width and 0 <= ny < height and rows[ny][nx] != '#']

    distances, pending = {start: 0}, deque([start])
    while pending:
        current = pending.popleft()
        for neighbour in neighbours(*current):
            if neighbour not in distances:
                distances[neighbour] = distances[current] + 1
                pending.append(neighbour)
    assert end in distances
    edges = sum(len(neighbours(*cell)) for cell in distances) // 2
    return {
        'width': width, 'height': height, 'exit': end,
        'open_cells': sum(tile != '#' for row in rows for tile in row),
        'reachable_cells': len(distances), 'shortest_distance': distances[end],
        'reachable_cycle': edges >= len(distances),
        'dead_ends': sum(len(neighbours(*cell)) == 1 for cell in distances
                         if cell not in {start, end}),
        'layout_sha256': hashlib.sha256(text.encode()).hexdigest(),
    }


makefile = (ROOT / 'Makefile').read_text()
sources = re.search(r'^SOURCES = (.+)$', makefile, re.M).group(1).split()
assert all((ROOT / source).is_file() for source in sources)
compiler = shlex.split(os.environ.get('CC', 'clang'))
flags = ['-std=c17', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-g', '-DMAZE_DRAWAPP']
executable = output / 'maze-audit'
run(compiler + flags + sources + ['graphics.c', '-o', str(executable)])
evidence = {'compiler': run(compiler + ['--version']).stdout.splitlines()[0]}
if shutil.which('gcc'):
    version = run(['gcc', '--version']).stdout.splitlines()[0]
    second = output / 'maze-gcc-driver'
    run(['gcc'] + flags + sources + ['graphics.c', '-o', str(second)])
    assert 'solved:' in run([second, '--headless']).stderr
    evidence['gcc_driver'] = version
    evidence['gcc_is_clang_alias'] = 'clang' in version.lower()

run(['make', 'test'])
run(['make', 'sanitize'])
run(['make', 'package'])

dimensions = [(5, 5), (7, 9), (15, 11), (21, 15), (35, 35), (101, 5), (5, 101), (101, 101)]
cases = []
for seed in range(20):
    width, height = dimensions[seed % len(dimensions)]
    loops = [0, 25, 50, 100][(seed // len(dimensions) + seed) % 4]
    options = ['--generate', str(width), str(height), '--seed', str(seed), '--loops', str(loops)]
    preview = run([executable, *options, '--preview']).stdout
    facts = inspect_grid(preview)
    assert facts['reachable_cells'] == facts['open_cells']
    assert run([executable, *options, '--preview']).stdout == preview
    result = run([executable, *options, '--headless', '--budget', str(20 * width * height)])
    match = re.search(r'solved: (\d+) moves, (\d+) turns, (\d+) actions;', result.stderr)
    assert match is not None
    moves, turns, actions = map(int, match.groups())
    assert moves + turns == actions <= 20 * width * height
    ex, ey = facts['exit']
    assert result.stdout.splitlines()[ey][ex] in '^>v<'
    cases.append(dict(facts, seed=seed, loops=loops, moves=moves, turns=turns, actions=actions))
assert len({case['layout_sha256'] for case in cases}) >= 2
evidence['generated_cases'] = cases
evidence['independent_fixtures'] = {
    path.name: inspect_grid(run([executable, '-f', path, '--preview']).stdout)
    for path in sorted((ROOT / 'mazes').glob('*.txt'))
}
assert evidence['independent_fixtures']['looped.txt']['reachable_cycle']
assert evidence['independent_fixtures']['branching.txt']['dead_ends'] > 0
evidence['source_sha256'] = {
    path.name: hashlib.sha256(path.read_bytes()).hexdigest()
    for path in sorted(ROOT.iterdir()) if path.suffix in {'.c', '.h'}
}
evidence['suites_passed'] = ['strict builds', 'make test', 'make sanitize', 'make package',
                             '20 additional deterministic generated cases', 'fixture topology']
(output / 'evidence.json').write_text(json.dumps(evidence, indent=2) + '\n')
print(f'Readiness checks passed: {len(cases)} additional generated cases; logs in {output}')
