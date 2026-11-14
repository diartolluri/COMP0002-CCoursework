"""Behaviour tests for the executable, its files, and argument validation."""
from collections import deque
from pathlib import Path
import re
import subprocess
import sys
import tempfile

PROGRAM = Path(sys.argv[1]).resolve()
ROOT = Path(__file__).resolve().parents[1]


def run(*arguments, status=0):
    result = subprocess.run([str(PROGRAM), *map(str, arguments)], capture_output=True,
                            text=True, timeout=20, cwd=ROOT)
    assert result.returncode == status, (arguments, result.returncode, result.stderr)
    return result


def check_grid(text):
    rows = text.splitlines()
    assert 5 <= len(rows) <= 101 and all(len(row) == len(rows[0]) for row in rows)
    width, height = len(rows[0]), len(rows)
    openings = [(x, y) for y, row in enumerate(rows) for x, tile in enumerate(row)
                if (x in (0, width - 1) or y in (0, height - 1)) and tile != '#']
    assert len(openings) == 2
    assert sum(row.count('S') for row in rows) == 1
    assert sum(row.count('E') for row in rows) == 1
    start = next((x, y) for y, row in enumerate(rows) for x, tile in enumerate(row) if tile == 'S')
    queue, seen = deque([start]), {start}
    while queue:
        x, y = queue.popleft()
        for dx, dy in ((0, -1), (1, 0), (0, 1), (-1, 0)):
            nx, ny = x + dx, y + dy
            if 0 <= nx < width and 0 <= ny < height and rows[ny][nx] != '#' and (nx, ny) not in seen:
                queue.append((nx, ny))
                seen.add((nx, ny))
    assert any(rows[y][x] == 'E' for x, y in seen)


assert 'Usage:' in run('--help').stdout
check_grid(run('--preview').stdout)
assert 'solved:' in run('--headless').stderr
assert 'solved:' in run('--headless', '--solver', 'wall').stderr
assert '48 actions;' in run('--headless').stderr
for preset in ['small', 'large', 'loops']:
    check_grid(run('--preset', preset, '--preview').stdout)
    assert 'solved:' in run('--preset', preset, '--headless').stderr
assert 'depth 16.' in run('--preset', 'small', '--solver', 'iddfs', '--headless').stderr
assert run('--preset', 'large', '--generate', '7', '5', '--preview').stdout == run('--generate', '7', '5', '--seed', '42', '--preview').stdout
assert run('--seed', '99', '--preset', 'loops', '--preview').stdout == run('--preset', 'loops', '--seed', '99', '--preview').stdout
assert run('--loops', '0', '--preset', 'loops', '--preview').stdout == run('--preset', 'loops', '--loops', '0', '--preview').stdout
assert run('--generate', '9', '7', '--preset', 'large', '--seed', '7', '--preview').stdout == run('--preset', 'large', '--seed', '7', '--generate', '9', '7', '--preview').stdout
for args in [('--preset',), ('--preset', 'invalid'), ('--preset', 'small', '--preset', 'large')]:
    assert run(*args, status=1).stdout == ''
for fixture in (ROOT / 'mazes').glob('*.txt'):
    check_grid(run('-f', fixture, '--preview').stdout)
    assert 'solved:' in run('-f', fixture, '--headless').stderr

for arguments in [('--unknown',), ('-f',), ('--generate', '8', '9'),
                  ('--generate', '5'), ('--generate', '103', '5'),
                  ('--delay', '-1'), ('--delay', '1001'), ('--budget', '1x'),
                  ('--seed', '3'), ('--loops', '101'), ('--solver', 'astar'),
                  ('--budget', '999999999999999999999999999999999'),
                  ('--generate', '9', '7', '-f', 'mazes/simple.txt'),
                  ('-f', 'does-not-exist.txt')]:
    result = run(*arguments, status=1)
    assert result.stdout == '' and result.stderr

limited = run('--headless', '--budget', '1', status=2)
assert 'budget exhausted' in limited.stderr and 'unsolvable' in limited.stderr
assert 'Graphics support is unavailable' in run('--graphics', status=1).stderr

with tempfile.TemporaryDirectory(prefix='maze-cli-') as temporary:
    directory = Path(temporary)
    saved = directory / 'roundtrip.txt'
    first = run('--generate', '9', '7', '--seed', '42', '--loops', '25', '--save', saved, '--preview')
    second = run('-f', saved, '--preview')
    assert first.stdout == second.stdout
    assert first.stdout == run('--generate', '9', '7', '--seed', '42', '--loops', '25', '--preview').stdout
    assert 'solved:' in run('-f', saved, '--headless').stderr
    assert 'solved:' in run('--generate', '9', '7', '--seed', '7', '--loops', '100', '--headless').stderr
    for size in (5, 15, 101):
        check_grid(run('--generate', size, size, '--seed', '4294967295', '--loops', '100', '--preview').stdout)

    valid = (ROOT / 'mazes/simple.txt').read_text()
    variants = [valid.replace('\n', '\r\n').encode(), valid.rstrip('\n').encode()]
    for i, data in enumerate(variants):
        path = directory / f'valid-{i}.txt'
        path.write_bytes(data)
        check_grid(run('-f', path, '--preview').stdout)
    malformed = [b'', b'#####\n#S.E#\n#####\n', valid.replace('S', '.').encode(),
                 valid.replace('E', '#').encode(), valid.replace('#', '?', 1).encode(),
                 valid.replace('S', '\x00').encode(), (valid + '\n').encode(),
                 valid.replace('\n', '\r').encode(), b'#' * 102 + b'\n',
                 ('S' + valid[1:]).encode()]
    for i, data in enumerate(malformed):
        path = directory / f'bad-{i}.txt'
        path.write_bytes(data)
        assert run('-f', path, '--preview', status=1).stdout == ''
    assert run('--save', directory / 'missing' / 'out.txt', '--preview', status=1).stdout == ''

    for name in ['--preset', '--headless', '--commands']:
        path = directory / name
        path.write_text(valid)
        check_grid(run('-f', path, '--preview').stdout)

print('CLI tests passed: fixtures, independent grid checks, round trips, malformed inputs, options, resource guards.')
