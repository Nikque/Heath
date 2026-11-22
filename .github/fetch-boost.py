"""Fetch the Boost headers for the workflows, where src/Makefile, src/NMakefile and src/Android*.mk look for them:
third_party/boost_1_92_0 next to the repository.

The paths stay relative this way: with an absolute BOOST_DIR the programs could keep the folder of the build machine
(tests/check-build-paths.py), and the Android makefiles do not read BOOST_DIR.
The archive is the official one. Its SHA-256 is the one published at URL + ".json": change both with the version.
usage: python .github/fetch-boost.py
"""
import hashlib
import subprocess
import sys
import tempfile
import zipfile
from pathlib import Path

NAME = 'boost_1_92_0'
URL = f'https://archives.boost.io/release/1.92.0/source/{NAME}.zip'
SHA256 = 'c46baee75ed4b558cf6553630d0615a147ed582812917b57e6f0839d1954549f'
THIRD_PARTY = Path(__file__).resolve().parents[2] / 'third_party'


def main():
    with tempfile.TemporaryDirectory() as tmp:
        archive = Path(tmp) / f'{NAME}.zip'
        subprocess.run(['curl', '--fail', '--location', '--silent', '--show-error', '--connect-timeout', '30',
                        '--max-time', '600', '--retry', '5', '--retry-all-errors', '--output', str(archive), URL],
                       check=True)
        digest = hashlib.sha256()
        with archive.open('rb') as f:
            while block := f.read(1 << 20):
                digest.update(block)
        if digest.hexdigest() != SHA256:
            sys.exit(f'{URL}: SHA-256 {digest.hexdigest()}, expected {SHA256}')
        with zipfile.ZipFile(archive) as z:
            # the headers and the license only: nothing of Boost is built
            members = [n for n in z.namelist()
                       if n.startswith(f'{NAME}/boost/') or n == f'{NAME}/LICENSE_1_0.txt']
            z.extractall(THIRD_PARTY, members)
    print(f'{THIRD_PARTY / NAME}: {len(members)} entries, SHA-256 of the archive checked')


if __name__ == '__main__':
    main()
