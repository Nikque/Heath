#!/usr/bin/env python3
"""Run cases of tests/regression on one logical CPU and keep their outputs.

usage: run_regression.py <regression.exe> <work dir> <eval.dat> <fixture.dat> [case ...]
Each case runs in <work dir> (with tests/merge-complete-fixture.dat copied there), with
EDAX_EVAL_FILE set, a time limit of 60 s (REGRESSION_TIMEOUT overrides it),
on logical CPU 0 (REGRESSION_CPU changes it),
and the output saved as <case>.out. A summary
line per case gives the exit code and the time.
"""

import os
import shutil
import subprocess
import sys
import time

CASES = ['bool', 'short', 'ggf', 'edx', 'savefail', 'deviatesave', 'expandredo', 'analyze', 'enhance',
         'core', 'solid-hash', 'nboard', 'gtp', 'gtptime', 'gtpreg', 'xboard', 'formats', 'pgnlong', 'pgnfen',
         'unsupported', 'bookio', 'truncated', 'corruptstartup', 'ggsclock', 'cassio', 'events',
         'missinglink', 'mergefile', 'linkstore', 'mergetwice', 'negamaxloop', 'learnline', 'hashlock',
         'lastgame', 'basereplace', 'keepunread', 'statsrange', 'gamescore', 'badinput', 'wthortrunc',
         'cassiolist', 'parsers', 'tools', 'unique', 'bookform2', 'evalform2', 'evalslots', 'crc32c',
         'bigcounts', 'bookread', 'mergehint', 'newform', 'evalmobility', 'evalform2mob', 'evalbadconv',
         'reproducible', 'taskzero', 'stabwindow', 'last-empty-diagonals', 'eval-vectors', 'stability-pair',
         'importscore', 'mergeorient', 'deviate254', 'deviateexport', 'configpath', 'histogram', 'gtpclock',
         'gtpregpass', 'ggsturn', 'gameio', 'optionsdump']


def main(argv):
    if len(argv) < 5:
        print(__doc__.strip())
        return 2
    exe, work, eval_file, fixture = (os.path.abspath(a) for a in argv[1:5])
    cases = argv[5:] or CASES
    missing = [p for p in (exe, eval_file, fixture) if not os.path.isfile(p)]
    if missing:
        print(f'missing: {", ".join(missing)}')
        return 2
    os.makedirs(os.path.join(work, 'tests'), exist_ok=True)
    shutil.copyfile(fixture, os.path.join(work, 'tests', 'merge-complete-fixture.dat'))
    env = dict(os.environ, EDAX_EVAL_FILE=eval_file)
    failed = 0
    skipped = 0
    for case in cases:
        t = time.time()
        with open(os.path.join(work, case + '.out'), 'wb') as out:
            p = subprocess.Popen([exe, case], cwd=work, env=env, stdout=out, stderr=subprocess.STDOUT)
            try:
                if os.name == 'nt':
                    import ctypes
                    h = ctypes.windll.kernel32.OpenProcess(0x0200 | 0x0400, False, p.pid)
                    cpu = int(os.environ.get('REGRESSION_CPU', '0'))
                    if not h or not ctypes.windll.kernel32.SetProcessAffinityMask(h, ctypes.c_size_t(1 << cpu)):
                        print(f'{case}: WARNING: the affinity to the CPU {cpu} failed: the case runs on any CPU')
                    if h:
                        ctypes.windll.kernel32.CloseHandle(h)
                code = p.wait(timeout=int(os.environ.get('REGRESSION_TIMEOUT', '60')))
            except subprocess.TimeoutExpired:
                p.kill()
                p.wait()
                code = 'timeout'
        if code == 77:
            skipped += 1
            print(f"{case}: skipped (unavailable in this build)")
            continue
        failed += code != 0
        print(f'{case}: exit {code}, {time.time() - t:.1f} s')
    print(f'{len(cases)} cases, {skipped} skipped, {failed} failed')
    return 0 if failed == 0 else 1


if __name__ == '__main__':
    sys.exit(main(sys.argv))
