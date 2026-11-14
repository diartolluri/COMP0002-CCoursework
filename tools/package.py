"""Build the source/data/readMe archive using an explicit allowlist."""
from pathlib import Path
from tempfile import NamedTemporaryFile
from zipfile import ZipFile, ZIP_DEFLATED

root = Path(__file__).resolve().parents[1]
names = (
    'main.c', 'config.c', 'config.h', 'maze.c', 'maze.h', 'generator.c',
    'generator.h', 'robot.c', 'robot.h', 'robot_internal.h', 'solver.c',
    'solver.h', 'render.c', 'render.h', 'README.md',
    'mazes/simple.txt', 'mazes/branching.txt', 'mazes/looped.txt',
    'mazes/north-entrance.txt', 'mazes/east-entrance.txt', 'mazes/south-entrance.txt',
)
for name in names:
    if not (root / name).is_file():
        raise SystemExit(f'Missing required package file: {name}')
output = root / 'build' / 'COMP0002CW1.zip'
output.parent.mkdir(exist_ok=True)
with NamedTemporaryFile(dir=output.parent, suffix='.zip', delete=False) as temporary:
    pending = Path(temporary.name)
try:
    with ZipFile(pending, 'w', compression=ZIP_DEFLATED) as archive:
        for name in names:
            archive.write(root / name, name)
    with ZipFile(pending) as archive:
        assert set(archive.namelist()) == set(names)
        assert archive.testzip() is None
        assert all(archive.read(name) == (root / name).read_bytes() for name in names)
    pending.replace(output)
finally:
    pending.unlink(missing_ok=True)
print(f'Created {output.name}: {len(names)} source/data/readMe files, {output.stat().st_size:,} bytes')
