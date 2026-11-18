"""Build and run an extracted source package using its actual README command."""
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import tempfile
from zipfile import ZipFile

ROOT = Path(__file__).resolve().parents[1]

with ZipFile(ROOT / 'build' / 'COMP0002CW1.zip') as archive:
    names = archive.namelist()
    assert len(names) == len(set(names)) == 21
    assert archive.testzip() is None
    assert all(name.endswith(('.c', '.h', '.txt')) or name == 'README.md' for name in names)
    assert not {'graphics.c', 'graphics.h', 'drawapp-5.0.jar'} & set(names)
    assert all(archive.read(name) == (ROOT / name).read_bytes() for name in names)
    readme = archive.read('README.md').decode()
    assert 'run.sh' not in readme and '.runtime' not in readme and '/Users/' not in readme
    assert len(readme.split()) <= 400, 'Keep the readMe concise enough for one page'
    commands = re.search(r'```sh\n(.*?)\n```', readme, re.S).group(1)
    compile_command = commands.split('\n./maze', 1)[0].replace('\\\n', ' ')

    with tempfile.TemporaryDirectory(prefix='maze-extracted-') as temporary:
        directory = Path(temporary)
        archive.extractall(directory)
        for name in ['graphics.h', 'graphics.c']:
            shutil.copy2(ROOT / name, directory / name)
        subprocess.run(shlex.split(compile_command), cwd=directory, check=True,
                       capture_output=True, text=True, timeout=30)

        def run(*arguments, status=0):
            result = subprocess.run([str(directory / 'maze'), *arguments], cwd=directory,
                                    capture_output=True, text=True, timeout=20)
            assert result.returncode == status, (arguments, result.stderr)
            return result

        assert run('--preset', 'large', '--delay', '0').stdout.startswith('SW ')
        assert 'solved:' in run('-f', 'mazes/looped.txt', '--headless').stderr
        original = run('--generate', '21', '15', '--seed', '99', '--save', 'example.txt', '--preview')
        assert original.stdout == run('-f', 'example.txt', '--preview').stdout
        assert 'solved:' in run('-f', 'example.txt', '--headless').stderr
        assert 'Usage:' in run('--help').stdout

print('Package checks passed: exact current sources, exclusions, README build, extracted runs.')
