"""Fetch an eval.dat of Heath (64 slots) for the tests of the workflows.

bin/data/eval.dat of the repository is the one of eval2: the builds read it with a warning, but their evaluation is then
wrong, and tests/libedax_test.c fails (book deviate adds nothing).
usage: python .github/fetch-eval.py <file to write>
"""
import hashlib
import subprocess
import sys
import tempfile
import zipfile
from pathlib import Path

URL = 'https://github.com/Nikque/Heath/releases/download/v4.5.5-heath.19/edax-4.5.5-heath.19-64.zip'
MEMBER = 'bin/data/eval.dat'
# of eval.dat itself: change it with URL
SHA256 = '6f8ceb1f9d8e8fcfc9c5117ace2b31f5f893ea54336d44e94fd74ee9e9bf1dbc'


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    target = Path(sys.argv[1])
    with tempfile.TemporaryDirectory() as tmp:
        archive = Path(tmp) / 'release.zip'
        subprocess.run(['curl', '--fail', '--location', '--silent', '--show-error', '--connect-timeout', '30',
                        '--max-time', '600', '--retry', '5', '--retry-all-errors', '--output', str(archive), URL],
                       check=True)
        with zipfile.ZipFile(archive) as z:
            data = z.read(MEMBER)
    digest = hashlib.sha256(data).hexdigest()
    if digest != SHA256:
        sys.exit(f'{URL}: SHA-256 of {MEMBER} {digest}, expected {SHA256}')
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_bytes(data)
    print(f'{target}: {len(data)} bytes, SHA-256 checked')


if __name__ == '__main__':
    main()
