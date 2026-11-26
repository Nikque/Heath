#!/usr/bin/env python3
"""Check the production 64-slot feature tables without changing their contents."""

import hashlib
from pathlib import Path
import sys

HEADER = Path(__file__).resolve().parents[2] / 'src' / 'eval' / 'eval_feature64.h'
TABLE_PREFIX = 'static const unsigned char EVAL_X_N'
TABLE_SHA256 = '9afe8351eb1b13ffa3e96d2b84634aafa2347e192a6434b218a59eb60fc0c32e'


def main(argv):
    if argv[1:] != ['check']:
        print('製品用の評価表は上書きしません。検査には check を指定してください。', file=sys.stderr)
        return 2
    try:
        _, marker, table = HEADER.read_text(encoding='utf-8').partition(TABLE_PREFIX)
    except OSError as err:
        print(err, file=sys.stderr)
        return 1
    matches = bool(marker) and hashlib.sha256(table.encode('utf-8')).hexdigest() == TABLE_SHA256
    print('評価表は登録値と一致します。' if matches else '評価表が登録値と異なります。')
    return 0 if matches else 1


if __name__ == '__main__':
    sys.exit(main(sys.argv))
