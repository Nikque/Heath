"""Check the pondering of the NBoard protocol (-ponder on) and -keep-hash (v4.5.5-newedax.14).

Run with Python 3 on Windows: nboard-ponder.py <Edax executable> <eval.dat> [--base <executable of v4.5.5-newedax.13>] [--cpus 0x3]
The checks, with 2 threads:
- with -ponder on -keep-hash on, as an interface that sends back with "move" the move of Edax's "go": Edax ponders
  (its process uses the CPU), "ping" does not stop the pondering, the opponent's move given by "set game" or by "move"
  goes on with the search pondered when it is the one guessed (the line of Edax's last search) and stops it when it
  is not; a "go" followed at once by a "ping" is answered at once; "set game" of the same game keeps the hash
  tables, of a new game cleans them; "set ponder off" stops the pondering;
- with -ponder off, and with no option: no pondering, and "set game" cleans the hash tables (as up to newedax.13);
- "set ponder on" and "set keep-hash on" switch them on;
- with --base: with no option, at depth 12 and 1 thread, the same moves and scores as v4.5.5-newedax.13.
Edax runs with -info, whose lines on the standard error tell what it does. The exit code is 1 when a check fails.
"""
import argparse, ctypes, ctypes.wintypes as wt, queue, random, re, subprocess, sys, tempfile, threading, time
from pathlib import Path

DIRS = [(-1, -1), (-1, 0), (-1, 1), (0, -1), (0, 1), (1, -1), (1, 0), (1, 1)]
k32 = ctypes.windll.kernel32
results = []


def flips(b, x, c):
    if b[x]:
        return []
    r, f, out = x // 8, x % 8, []
    for dr, df in DIRS:
        line, rr, ff = [], r + dr, f + df
        while 0 <= rr < 8 and 0 <= ff < 8 and b[rr * 8 + ff] == 3 - c:
            line.append(rr * 8 + ff)
            rr, ff = rr + dr, ff + df
        if line and 0 <= rr < 8 and 0 <= ff < 8 and b[rr * 8 + ff] == c:
            out += line
    return out


def legal(b, c):
    return [x for x in range(64) if flips(b, x, c)]


def play(b, x, c):
    b = b[:]
    for y in flips(b, x, c):
        b[y] = c
    b[x] = c
    return b


def sq(x):
    return 'abcdefgh'[x % 8] + str(x // 8 + 1)


def idx(t):
    return 'abcdefgh'.index(t[0].lower()) + 8 * (int(t[1]) - 1)


def ggf(b, c):
    return '(;GM[Othello]PC[test]PB[]PW[]RE[?]TI[0:00]TY[8]BO[8 %s %s];)' % (''.join('-*O'[v] for v in b), '*' if c == 1 else 'O')


def random_position(seed, n):
    rnd = random.Random(seed)
    b, c = [0] * 64, 1
    b[27] = b[36] = 2
    b[28] = b[35] = 1
    for _ in range(n):
        m = legal(b, c)
        if not m:
            c = 3 - c
            m = legal(b, c)
        b = play(b, rnd.choice(m), c)
        c = 3 - c
    if not legal(b, c):
        c = 3 - c
    return b, c


class FT(ctypes.Structure):
    _fields_ = [('lo', wt.DWORD), ('hi', wt.DWORD)]


class Engine:
    def __init__(self, exe, extra, tasks=2, log=None, info=True):
        args = [str(exe), '-nboard', '-eval-file', str(EVAL), '-n-tasks', str(tasks), '-book-usage', 'off'] + list(extra)
        if info:
            args.append('-info')
        if log:
            args += ['-ui-log-file', str(log)]
        self.p = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, bufsize=1)
        k32.SetProcessAffinityMask(wt.HANDLE(int(self.p._handle)), CPUS)
        self.out, self.err, self.ping = queue.Queue(), [], 0
        threading.Thread(target=self._read, daemon=True).start()
        threading.Thread(target=self._read_err, daemon=True).start()
        self.send('nboard 1')

    def _read(self):
        for line in self.p.stdout:
            self.out.put(line.rstrip('\n'))

    def _read_err(self):
        for line in self.p.stderr:
            self.err.append((time.monotonic(), line.rstrip('\n')))

    def send(self, s):
        self.p.stdin.write(s + '\n')
        self.p.stdin.flush()

    def until(self, pred, timeout=30):
        end = time.monotonic() + timeout
        while True:
            t = end - time.monotonic()
            if t <= 0:
                raise TimeoutError('no answer')
            r = pred(self.out.get(timeout=t))
            if r is not None:
                return r

    def sync(self):
        self.ping += 1
        tag = 'pong %d' % self.ping
        t = time.monotonic()
        self.send('ping %d' % self.ping)
        self.until(lambda l: True if l == tag else None)
        return time.monotonic() - t

    def go(self, think):
        """go, then a ping after think seconds: the move and the seconds to the answer."""
        t0 = time.monotonic()
        self.send('go')
        answer = lambda l: l[3:].strip() if l.startswith('===') else None
        try:
            r = self.until(answer, think)
        except (TimeoutError, queue.Empty):
            self.ping += 1
            self.send('ping %d' % self.ping)
            r = self.until(answer, 10)
        return r.split()[0], time.monotonic() - t0

    def cpu(self):
        a, b, c, d = FT(), FT(), FT(), FT()
        k32.GetProcessTimes(wt.HANDLE(int(self.p._handle)), ctypes.byref(a), ctypes.byref(b), ctypes.byref(c), ctypes.byref(d))
        return ((c.hi << 32 | c.lo) + (d.hi << 32 | d.lo)) / 1e7

    def err_since(self, t):
        return [l for (s, l) in self.err if s >= t]

    def quit(self):
        try:
            self.send('quit')
            self.p.wait(10)
        except Exception:
            self.p.kill()


def count(lines, text):
    return sum(text in l for l in lines)


def last_pv(log):
    pv = None
    for line in open(log, encoding='latin-1'):
        if line.startswith('edax>'):
            pv = re.findall(r'\b([a-hA-H][1-8])\b', line)
    return pv


def check(name, ok, detail=''):
    results.append((name, ok))
    print(('ok    ' if ok else 'FAIL  ') + name + ('  (' + detail + ')' if detail else ''), flush=True)


def turn(e, b, c, think=1.0):
    t = time.monotonic()
    e.sync()
    e.send('set game ' + ggf(b, c))
    m, dt = e.go(think)
    return m, dt, e.err_since(t)


def guessed(b, c, pv):
    moves = legal(b, c)
    g = idx(pv[1]) if pv and len(pv) > 1 else None
    return g if g in moves else None


def pondering(tmp):
    log = tmp / 'nboard.log'
    e = Engine(EXE, ['-ponder', 'on', '-keep-hash', 'on'], 2, log)
    e.send('set depth 60')
    b, c = random_position(7, 10)
    hit = None
    for n in range(8):
        fast = n in (5, 6)
        m, dt, errs = turn(e, b, c, 0.0 if fast else 1.0)
        x = idx(m)
        check('%d: a legal move' % n, x in legal(b, c), m)
        if n:
            check('%d: set game of the game keeps the hash tables' % n, count(errs, '< cleaning hashtable (base') == 0)
            switched, stopped = count(errs, '[switch from pondering to thinking') > 0, count(errs, '[stop pondering]') > 0
            if hit:
                check('%d: the move guessed goes on with the pondering' % n, switched)
            elif hit is False:
                check('%d: another move stops the pondering' % n, stopped and not switched)
        if fast:
            check('%d: go and ping at once: answered at once' % n, dt < 1.0, '%.2f s' % dt)
        pv = last_pv(log)
        b = play(b, x, c)
        t = time.monotonic()
        c0 = e.cpu()
        e.send('move ' + m)
        time.sleep(0.8)
        busy = e.cpu() - c0
        e.sync()
        k = e.ping
        c1 = e.cpu()
        time.sleep(0.5)
        after = e.cpu() - c1
        check('%d: ponders after its own move' % n, count(e.err_since(t), '[start ponderation]') == 1 and busy > 0.2, 'cpu %.2f s in 0.8 s' % busy)
        lines = [l for _, l in e.err]
        i = next(i for i, l in enumerate(lines) if l.startswith('<event add') and l.endswith(': ping %d>' % k))
        check('%d: ping does not stop the pondering' % n, '[stop on user demand]' not in lines[i - 1] and after > 0.1, 'cpu %.2f s in 0.5 s' % after)
        moves = legal(b, 3 - c)
        if not moves:
            hit = None
            continue
        g = guessed(b, 3 - c, pv)
        if n % 2 == 0 and g is not None:
            o, hit = g, True
        else:
            o = next((y for y in moves if y != g), moves[0])
            hit = None if o == g else False
        b = play(b, o, 3 - c)
        if not legal(b, c):
            break
    # the opponent's move by "move", then "go"
    m, dt, errs = turn(e, b, c)
    pv = last_pv(log)
    b = play(b, idx(m), c)
    e.send('move ' + m)
    time.sleep(0.5)
    g = guessed(b, 3 - c, pv)
    if g is not None:
        t = time.monotonic()
        e.send('move ' + sq(g))
        mm, dt = e.go(1.0)
        b = play(b, g, 3 - c)
        check('move then go: a legal move', idx(mm) in legal(b, c), mm)
        errs = e.err_since(t)
        check('move then go: the move guessed goes on with the pondering', count(errs, '[switch from pondering to thinking') > 0,
              'guessed %s, stop pondering %d' % (sq(g), count(errs, '[stop pondering]')))
    nb, nc = random_position(99, 6)
    t = time.monotonic()
    e.sync()
    e.send('set game ' + ggf(nb, nc))
    e.sync()
    check('set game of a new game cleans the hash tables', count(e.err_since(t), '< cleaning hashtable (base') == 3)
    m, dt, errs = turn(e, nb, nc)
    e.send('move ' + m)
    time.sleep(0.5)
    t = time.monotonic()
    e.send('set ponder off')
    e.sync()
    time.sleep(0.3)
    c0 = e.cpu()
    time.sleep(0.5)
    check('set ponder off stops the pondering', count(e.err_since(t), '[stop pondering]') > 0 and e.cpu() - c0 < 0.1)
    e.quit()


def no_pondering(name, extra):
    e = Engine(EXE, extra)
    e.send('set depth 60')
    b, c = random_position(11, 14)
    for n in range(3):
        m, dt, errs = turn(e, b, c, 0.5)
        if n:
            check('%s %d: set game cleans the hash tables' % (name, n), count(errs, '< cleaning hashtable (base') == 3)
        b = play(b, idx(m), c)
        t = time.monotonic()
        c0 = e.cpu()
        e.send('move ' + m)
        time.sleep(1.0)
        check('%s %d: no pondering' % (name, n), count(e.err_since(t), 'ponderation') == 0 and e.cpu() - c0 < 0.1, 'cpu %.2f s' % (e.cpu() - c0))
        o = legal(b, 3 - c)
        if o:
            b = play(b, o[0], 3 - c)
    e.quit()


def switches():
    e = Engine(EXE, [])
    e.send('set depth 60')
    e.send('set ponder on')
    e.send('set keep-hash on')
    b, c = random_position(13, 12)
    m, dt, errs = turn(e, b, c, 0.5)
    b = play(b, idx(m), c)
    t = time.monotonic()
    e.send('move ' + m)
    time.sleep(0.8)
    check('set ponder on: ponders', count(e.err_since(t), '[start ponderation]') == 1)
    b = play(b, legal(b, 3 - c)[-1], 3 - c)
    m, dt, errs = turn(e, b, c, 0.5)
    check('set keep-hash on: keeps the hash tables', count(errs, '< cleaning hashtable (base') == 0)
    e.quit()


def same_as_base(base):
    rows = []
    for exe in (base, EXE):
        e = Engine(exe, [], 1, None, info=False)
        e.send('set depth 12')
        b, c = random_position(21, 8)
        out = []
        for _ in range(14):
            e.sync()
            e.send('set game ' + ggf(b, c))
            e.send('go')
            r = e.until(lambda l: l[3:].strip() if l.startswith('===') else None, 60).split()
            out.append((r[0], r[1]))
            b, c = play(b, idx(r[0]), c), 3 - c
            if not legal(b, c):
                c = 3 - c
                if not legal(b, c):
                    break
        e.quit()
        rows.append(out)
    check('no option, depth 12: the moves and scores of the base', rows[0] == rows[1], ' '.join('%s/%s' % r for r in rows[1]))


if __name__ == '__main__':
    a = argparse.ArgumentParser()
    a.add_argument('exe')
    a.add_argument('eval')
    a.add_argument('--base')
    a.add_argument('--cpus', default='0x3')
    o = a.parse_args()
    EXE, EVAL, CPUS = Path(o.exe), Path(o.eval), int(o.cpus, 16)
    with tempfile.TemporaryDirectory() as d:
        pondering(Path(d))
    no_pondering('-ponder off', ['-ponder', 'off', '-keep-hash', 'off'])
    no_pondering('no option', [])
    switches()
    if o.base:
        same_as_base(o.base)
    bad = [n for n, ok in results if not ok]
    print('%d checks, %d failed' % (len(results), len(bad)))
    sys.exit(1 if bad else 0)
