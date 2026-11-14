"""Check launcher routing with real C builds and a test-only Java process."""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]

with tempfile.TemporaryDirectory(prefix='maze-launcher-') as temporary:
    project = Path(temporary)
    shutil.copy2(ROOT / 'run.sh', project / 'run.sh')
    (project / 'build').symlink_to(ROOT / 'build', target_is_directory=True)
    (project / 'drawapp-5.0.jar').touch()
    tools = project / 'tools'
    (tools / 'bin').mkdir(parents=True)
    make = tools / 'make'
    make.write_text('#!/bin/sh\nexit 0\n')
    make.chmod(0o755)
    java = tools / 'bin' / 'java'
    java.write_text('''#!/bin/sh
if [ "$1" = "-version" ]; then exit 0; fi
cat > /dev/null
printf 'VIEWER_STARTED\\n'
''')
    java.chmod(0o755)
    environment = dict(os.environ, PATH=f'{tools}{os.pathsep}{os.environ["PATH"]}',
                       JAVA_HOME=str(tools))

    def run(*arguments, status=0):
        result = subprocess.run([str(project / 'run.sh'), *arguments], cwd=temporary,
                                env=environment, text=True, capture_output=True, timeout=20)
        assert result.returncode == status, (arguments, result.stderr)
        return result

    assert run('--delay', '0').stdout == 'VIEWER_STARTED\n'
    assert 'Usage:' in run('--help').stdout
    assert 'solved:' in run('--headless').stderr
    assert 'S' in run('--preview').stdout
    commands = run('--commands', '--delay', '0')
    assert commands.stdout.startswith('SW ') and 'VIEWER_STARTED' not in commands.stdout

    valid = (ROOT / 'mazes' / 'simple.txt').read_text()
    for name in ['--headless', '--preview', '--commands', '--preset']:
        (project / name).write_text(valid)
        assert run('-f', name, '--delay', '0').stdout == 'VIEWER_STARTED\n', name
    (project / '--preview').unlink()
    saved = run('--save', '--preview', '--delay', '0')
    assert saved.stdout == 'VIEWER_STARTED\n' and (project / '--preview').is_file()
    assert '--generate requires' in run('--generate', '9', '--headless', status=1).stderr
    assert '--solver must' in run('--solver', '--headless', status=1).stderr

print('Launcher checks passed: mode routing, option-like filenames, malformed arguments.')
