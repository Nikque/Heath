# Heath (formerly newedax) release notes

[日本語](RELEASE-NOTES.ja.md) · [README](README.md) · [Manual](manual/en/README.md) · [Tests and measurements](docs/validation/newedax.en.md)

User-facing changes of each version of Heath (formerly newedax: the eval2 line of the C Edax 4.5.5 nikque, ported to C++23). Detailed test and measurement records are linked at the end of each entry.

For the details of changes taken from the C version, see the [C release notes](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.md).

## Relation to the C versions

The table lists corresponding releases. Fixes to shared functionality are carried into the respective implementation.

| Heath (newedax) | C (nikque) | C (eval2) |
|---|---|---|
| heath.21 | nikque.34 | eval2.21 |
| heath.20 | nikque.33 | eval2.20 |
| heath.20-pre1 | nikque.33-pre1 | eval2.20-pre1 |
| heath.19 | nikque.32 | eval2.19 |
| heath.18 | nikque.31 | eval2.19 |
| heath.17 | nikque.30 | eval2.18 |
| heath.16 | nikque.29 | eval2.17 |
| newedax.15 | nikque.28 | eval2.16 |
| newedax.14 | nikque.27 | eval2.15 |
| newedax.13 | nikque.27 | eval2.15 |
| newedax.12 | nikque.27 | eval2.15 |
| newedax.11 | nikque.26 | eval2.14 |
| newedax.10 | nikque.25 | eval2.13 |
| newedax.9 | nikque.24 | eval2.12 |
| newedax.8 | nikque.23 | eval2.11 |
| newedax.7 | nikque.22 | eval2.10 |
| newedax.6 | nikque.21 | eval2.9 |
| newedax.5 | nikque.20 | eval2.8 |
| newedax.4 | nikque.19 | eval2.7 |
| newedax.3 | nikque.18 | eval2.6 |
| newedax.2 | nikque.18 | eval2.6 |
| newedax.1 | nikque.17 | eval2.5 |

---

## v4.5.5-heath.21

This maintenance release retains the respective evaluation weights for the 64-slot and 48-slot builds.

In the 64-slot and 48-slot ZIPs (`edax-4.5.5-heath.21-64.zip`, `edax-4.5.5-heath.21-48.zip`), all 5 Windows executables (`wEdax-x86-64-v4.exe`, `wEdax-x86-64-v3.exe`, `wEdax-x86-64.exe`, `wEdax-x86-sse.exe`, `wEdax-x86.exe`) and the 3 DLLs are PGO builds. The release ZIPs were replaced with these on 2026-11-26. In ZIPs downloaded before that, `wEdax-x86-64-v4.exe` and the 3 DLLs are built without PGO. The supplemental ZIPs that were briefly available (`edax-4.5.5-heath.21-64-pgo.zip`, `edax-4.5.5-heath.21-48-docs1.zip`) have been withdrawn. The speed difference from the PGO builds was not measured.

### Search and libedax

- Search stops requested immediately before the search starts are retained. Pondering is stopped before the next game resets its search state. Parallel searches synchronize access to their shared bounds, recheck the window before starting a child search, and synchronize the completion of lazy helpers. Transposition-table reads use relaxed atomics for the fields read outside the write lock.
- Changing the thread count rebuilds the link to task zero and accepts the maximum supported thread count. The GGS transition back to one game retains a task for the second search. A numeric lazy setting no longer starts helpers when the move list is restricted.
- Equal aspiration bounds are widened. Stable-disc bounds narrow a search window when the two ranges overlap. Invalid or non-finite ProbCut depth settings fall back to the default, and probes outside the usable depth range are skipped.
- A book hit is recorded only when a legal book move is available. The Roxane move generator now handles the four centre squares using the generic flip routine.
- In libedax, a stop just before a hint or benchmark search starts is retained. A cancelled hint keeps only completed results. Asking for the next hint returns `LIBEDAX_NOMOVE`. Interrupted benchmark positions are excluded from the totals.

### Opening books

- New books initialize the saved statistics, pending work and display settings. Serial and parallel `book negamax` mark the book for saving when stored values, bounds or statistics change. If adding a child fails, the link to that child is removed.
- `book fix` reserves space before moving positions into their canonical buckets. If that reservation fails, it reports the incomplete repair and still removes links to absent positions, checking all eight symmetries before discarding a link.
- `book learn` checks the game filename and previously processed lines against a fingerprint covering the saved boards, links, scores, bounds, statistics and marks. Old progress without sufficient identity is ignored with a warning. A read error leaves progress available for a later run.
- If progress cannot be written before saving, the engine invalidates the old `.learn` progress and warns before updating `.store`. If necessary, `.learn.invalid` prevents stale progress from being reused on restart. If neither the marker nor removal is possible, it preserves the prior `.store` and tries `.store.recovery`.
- Book loading recognizes a continuation only for the expected suffixes. An unrelated file with the same prefix does not acquire that status. If a protected filename or failed-load marker cannot be retained, the engine stops the relevant automatic save or replacement. The second pass of a merge checks the input's checksum, length and position count and rolls back added positions if the input changed. A failed replacement restores the old book when it can be put back.
- Text writes are checked through stream close before replacement. Backups such as `.old` and `.damaged` prefer exclusive rename or link operations. The fallback checks that the destination is absent before renaming. Another process can create that name during the gap.
- Book analysis reports overlong arguments and setup failures with the request ID. It reads queued positions at the correct width on 32-bit systems and retries game-count calculation after temporary allocation failure.
- `book load` reports `@load ok=`. Automatic book saves on exit report `@save ok=` and return a failure exit status if the save fails. Settings show book depth and format independently.

### Game files, OBF and opening names

- SGF and GGF readers detect games cut short. SGF reads the first variation, round-trips custom starting positions and turns through `AB`, `AW` and `PL`, and supports a UTF-8 BOM. GGF checks move colours and explicit passes, accepts omitted forced passes, and checks date fields. NBoard applies a game only after it is fully validated.
- TXT game files can use a UTF-8 BOM and round-trip a custom starting board and turn. Extraction can continue past a malformed line. Operations such as `base correct` preserve the existing output when the input is incomplete. Game hashes are recomputed after moves are supplemented or replaced.
- PGN uses the score from Black's perspective even when White starts, and writes `*` for unfinished games. PGN numerical fields are checked. OBF and FEN exports use the actual starting board, starting turn and forced passes.
- WThor append preserves the stored year and depth. WThor diagnostics handle out-of-range depth and use the actual number of empty squares for search settings. Companion filenames are checked before use.
- OBF checks board validity before searching. Selection uses an exclusively created temporary file beside the destination and closes both streams before replacing the destination, including in-place selection and file aliases. Input and output failures return a nonzero CLI status or an `ERROR` line in the prompt, where later commands remain available. Results keep the full input path and include wrong scores even when the best move is correct.
- Duplicate opening lines are removed from same-name lookup, which supports `name #n`. Multiple names for a single position are displayed together.

### Protocols

- Input handling preserves a final line without a newline and gives Cassio sole ownership of input after a protocol switch. Thread-start and event-allocation failures are diagnosed. Cassio preserves stop requests across event and search-state transitions and keeps the search window non-empty.
- GTP validates every `time_settings` field and converts seconds with a wide integer. `reg_genmove` restores the board, turn, clocks and book random state, so a trial does not change the subsequent book choice.
- XBoard `playother` takes the side opposite the current turn. `go` after game over reports only the result. Leaving analysis returns to the command loop, and quit or EOF exits both loops.
- GGS checks whose turn it is, processes complete lines at disconnect, discards an unterminated fragment and exits. Authentication values are masked in command logs, settings and diagnostics. The console `count` command accepts 6×6 and 8×8 boards.

### Evaluation and tools

- The engine distinguishes the 64-slot and 48-slot weight layouts. A 48-slot build refuses weights with nonzero omitted groups. Legacy eval2 weights retain the existing warning-and-load behavior. Format-2 coefficients are checked for finite values and excessive error terms. Conversion writes through an exclusive temporary file and preserves the old destination if it fails.
- `eval_builder` maps 12-square patterns to the correct coefficient table, keeps training scores signed, validates passes, compares coefficients when only one side is zero, and fails on a read error partway through a game file.
- `evsearch` and `evtrain` fail before writing a model when required inputs contain no usable records. Calibration requires validation data for each relevant stage. Record readers check file boundaries, short reads and counts, and learning reduces only the threads actually started by OpenMP. The engine and evsearch return a failing exit status for unknown options.
- `gen`, `pcsigma`, `leafgen` and `playout` validate input records and boards before handing them to search. `sample` bounds allocations by the available records. Sampling and labelling validate large counts, and labelling synchronizes its progress counter.
- The evaluation tools reject truncated format-1 weights and extra data. `w2dat2` checks interpolation paths, combines both mobility terms and rounds and clamps to the 16-bit range. `sprepair` uses an exclusive temporary output. The 64-slot table check validates the production layout without overwriting it with the experimental table.

### Build, tests and documentation

- The flip-table generators build with MSVC, return failure on write or close errors, and generate files where `make code` actually includes them. The standalone move-generation harness compares legal moves and flips with an independent implementation. Benchmark counters and empty/single-value histograms are handled without overflow or division by zero.
- Regression tests exercise stop, search windows, input and save behavior. The stable-disc test uses reference scores for 1,024 windows and can independently recompute those scores. The exact FFO checker rejects duplicate result rows and score bounds. Restart tests require a successful exit and use EOF to finish the queued commands. DLL compatibility mode is opt-in, and evaluation-tool tests fail when required or explicitly requested checks are skipped.
- Release automation checks named artifacts and their build provenance and validates the evaluation pattern definitions, coefficients, checksum and file end before packaging. The CI and QEMU checks verify solved positions and surface command failures. Benchmark and comparison scripts reject missing tools and failed or empty runs.
- The manuals explain the Windows and Android public ZIPs, CPU requirements, evaluation weights, book formats and text progress files, and supported game-file round trips. Published memory figures now cite the measured configuration. The source-restructuring tool refuses trees already in the current layout.

[Validation record](docs/validation/newedax.en.md#v455-heath21)

---

## v4.5.5-heath.20

The release of v4.5.5-heath.20-pre1 with a better search and lazy auto. The time of a move and the probcut change. In games of one thread, the changes of the search won more discs than the search before (see Strength).

**Changes**

- All the changes of v4.5.5-heath.20-pre1.
- Five changes of the search. They are always on and cannot be turned off. Two of their values can still be set: `time-ext-max` (2.0, how far the time of a move may stretch) and `pc-end` (0.85, the factor of the margin of the probcut of the searches to the end).

| What changes | How |
|---|---|
| Time of a move in a game | The time of Edax a move. Only after a fail low, it may stretch up to 2 times |
| Order of the moves | Sorted with the evaluation from the depth 7; the weight shifts of the evaluation are 1,0,0 |
| Probcut off the principal variation | Narrower (2,0.04,0.75) |
| Probcut by the mobility | The margin depends on the mobility (2.219,-0.184,-0.350) |
| Probcut of the searches to the end | A shallow score is read 1.3 times, up to 12 discs; the margin is 0.85 times |

- The default of `lazy` is `auto`: one helper thread searches the root position deeper in a midgame search of at least 4 threads. It is not used to solve the end of the game, in a selective search to the end, or with 3 threads or fewer.
- The test options are deleted: `-corr`, `-corr-end`, `-time-model`, `-time-ext-hard`, `-time-sep`, `-time-sep-depth`, `-time-close` and `-time-close-empties`.
- As the five changes are fixed, their options are deleted: `-time-ext`, `-sort-eval-shift`, `-sort-min-depth`, `-pc-slope`, `-pc-dev` and `-pc-mob`. `-pc-end` stays; its default is 0.85.
- The probcut model is fixed to `refit`, and its setting `probcut-model` (`-probcut-model`, and `probcut-model` in `config.ini`) is deleted. `refit` is the default since v4.5.5-newedax.13, so the search results do not change. `standard` can no longer be used.

**Strength**

One thread (4 threads for lazy), no book, both colors from positions filled at random; the time is that of each side. The games were played with a build that had the changes as options; this version searches the same scores, nodes and moves without them (the tests and measurements). The first two rows compare the changes one after the other, not this version with v4.5.5-heath.19 directly, and lazy auto was not shown stronger.

| Comparison | Time a game | Games | Discs a game | Elo |
|---|---|---|---|---|
| Probcut of the searches to the end on / off | 30 s | 4,000 | +0.140 [+0.082, +0.197] | +1.4 |
| All five / the probcut of the searches to the end alone | 180 s | 2,000 | +0.181 [+0.102, +0.260] | +2.6 |
| lazy auto on / off (4 threads) | 30 s | 2,000 | +0.084 [-0.015, +0.183] | -0.2 |

**Compatibility**

- The search results (scores, nodes, moves) differ from the version before: the probcut and the order of the moves change.
- A book learned with this version may have other scores than one learned with the version before.
- The `eval.dat` and the book file format are the same as in the version before.

**Notes**

- With a deleted option on the command line, the program shows its usage and ends. Fix the scripts that pass one.

**Not checked**: the effect of the five changes was measured only in games of 180 s with one thread; not with several threads or shorter times.

[Tests and measurements](docs/validation/newedax.en.md#v455-heath20)

---

## v4.5.5-heath.20-pre1

A pre-release with the book and base functions of Edax 4.7, new search settings and fixes.

**Changes**

- The book and base functions of Edax 4.7 and new search settings were added, as in [v4.5.5-nikque.33-pre1](https://github.com/Nikque/edax-reversi-AVX/blob/v4.5.5-nikque.33-pre1/RELEASE-NOTES.md#v455-nikque33-pre1).

**Fixes**

- In a timed search with several threads, the whole search could stop just after its time had been extended. Split searches now use the extended time.
- Reading a `.pgn` with a pass (`pass`) could lose the rest of that game and the next game. Passes are now read.
- `stop` of the standard input, and `?` of XBoard, could be lost when they arrived just before a search began. They now stop it. This fix does not cover GGS or `edax_stop` of the DLL.
- Starting a new game while pondering waited for the pondering to end. The pondering is now stopped first.
- `book learn` could fail to continue after a stop around a save to `.store` (since v4.5.5-nikque.29, eval2.17 and heath.16). It can now continue wherever it stops. A `.learn` written by the previous versions can still be used.
- In NBoard, a `stop` that arrived while a `go` waited made every later move use the least time. The gap left by the earlier fix of `ping` is also closed.
- Non-ASCII file names could be garbled by `book deviate export` and others in the DLL. UTF-8 names are now used.
- Only one of the four bytes of `book_move` and `need_saving` was written by the Windows DLL. They now have four bytes, as in the C version.
- The other fixes of the book and base functions are those of [v4.5.5-nikque.33-pre1](https://github.com/Nikque/edax-reversi-AVX/blob/v4.5.5-nikque.33-pre1/RELEASE-NOTES.md#v455-nikque33-pre1).

**Compatibility**

- The previous release's `eval.dat` and books remain compatible. The book file format is unchanged.
- With the default settings, searches with a given level or depth give the same scores and best moves as the previous version.
- Timed searches with several threads can give different results because of the fix.
- `book play` and `base complete` can give results different from the previous version when they run in parallel.
- Books made by the `book deviate` family, `book enhance` and `book leaf expand` can differ slightly from the previous version, as the repeated searches are gone.
- The lines written by `book extract` end where they join, so they can be shorter and fewer than before.
- `book play` can give other values to positions that cannot be reached from the initial position.
- `book play` no longer saves at every round. An interrupted run can lose up to `book-save-interval` (60 minutes by default).
- Saving games needs free space for two copies of the file.
- `book play n` creates a `.play-to` file and `base complete` a file with `.completing` for their progress. Do not delete them before resuming.
- `book play` without a number, `book extract`, `base convert` and `base unique` keep their previous meaning, which differs from Edax 4.7.

**Notes**

- This is a pre-release for testing. Setting names and defaults may change before the final version.
- Keep a copy of important books before use.
- To continue `book learn`, load `.store` with `book load` first. Without this, `.learn` is overwritten and it may no longer be possible to continue from that `.store`.

**Not checked**: the time settings, `-split-probe` alone, playing strength, 32 threads, large production books, Android devices, the Linux and macOS builds, and use from a GUI.

[Tests and measurements](docs/validation/newedax.en.md#v455-heath20-pre1)

---

## v4.5.5-heath.19

The exact endgame search is faster. The change is the same as in [v4.5.5-nikque.32](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.md#v455-nikque32).

**Speed**

- At 5 and 6 empty squares, a quadrant's only empty square is played first, and a small thread-local transposition table (2^14 entries) is probed. The idea comes from KUROOBI (MIT).
- With one thread, exact solves of fforum-40-59 take 98.2% of the time (505.2 s to 496.2 s: the average of 4 runs each, alternately, on one logical CPU of a Ryzen 9 9950X while other work used every CPU). The positions searched: 90.2% on fforum-40-59 (28.63 to 25.81 billion), 89.0% on fforum-20-39. Every score and best move is the same as in the previous release.
- The midgame search is unchanged: positions that are not solved search the same number of positions as before.

**Compatibility**

- The previous release's `eval.dat`, books, and search and learning settings remain compatible.

**Not checked.** The speed with many threads (such as 32), production books, and playing strength.

[Tests and measurements](docs/validation/newedax.en.md#v455-heath19)

---

## v4.5.5-heath.18

This release includes the seven fixes from [v4.5.5-nikque.31](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.md#v455-nikque31) for database summaries, estimate errors, command help, and external protocols.

**Fixes**

- Fixed the GTP response at game end and the handling of `q` in XBoard, GTP, and NBoard. GGS now handles the end of standard input locally.

**Compatibility**

- The previous release's `eval.dat`, books, and search and learning settings remain compatible.

**Not checked.** Production books, playing strength, and communication with a live GGS server.

[Tests and measurements](docs/validation/newedax.en.md#v455-heath18)

---

## v4.5.5-heath.17

This release includes the changes from [v4.5.5-nikque.30](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.md#v455-nikque30). The SVE evaluation update supports both the 64-slot and 48-slot builds.

**Fixes**

- Normalizing the color of solid discs in an endgame transposition-table key could produce an invalid exact-search bound. When the winner changes, empty squares are awarded to the other side. The key now uses the actual board.

**Compatibility**

- The fix can change search results or node counts in affected positions. The evaluation function, `eval.dat`, and the book file format are the same as in the previous release.

**Not checked**. Playing strength, a production book, and SVE behavior and speed on hardware.

[Tests and measurements](docs/validation/newedax.en.md#v455-heath17)

---

## v4.5.5-heath.16

The first version under the name Heath (formerly newedax). `book correct`, `book leaf recalculate` and `book learn` can go on from a book saved before they ended, and `book deepen` now works.

**Changes**

- The changes of the 4 book commands are those of the C versions v4.5.5-nikque.29 and v4.5.5-eval2.17 ([the C release notes](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.md#v455-nikque29)).
- `book correct` marks in the book each solved position it searched again.
- Run again on a book saved before it ended (`<book-file>.err`, or the save at the exit), `book correct` searches only the positions without a mark. When every position is done, it removes all the marks.
- Up to v4.5.5-newedax.15, a new run of `book correct` searched every solved position again from the beginning (days on a large book).
- `book leaf recalculate` (2, 3, 4) does the same with the leaves it searches again, and goes on from where it stopped.
- `book learn <file>` writes `<book-file>.learn`. It holds the number of lines of the file learned in the book saved to `<book-file>.store`, a check value of them, and the number of positions, the level and the depth of the book.
- `book learn` run again with the same file on this book skips the lines already learned. `<book-file>.learn` is removed when the whole file is learned.
- `book deepen` gives the positions of a level shallower than the book the level of the book, and searches them again with it. Before, it searched them again with their own level: nothing became deeper, and a new run did all again.
- The name: Heath (formerly newedax) in README, the release notes and the manual. The repository is now `Nikque/Heath` (the old address leads there).
- The program still calls itself Edax (in the protocols used by GUIs). The names of the earlier versions (`v4.5.5-newedax.N`) do not change.

**Compatibility**

- The marks are the two upper bits of the level byte of a position, in both forms of book file. The size and the format of the file do not change. A book without marks is the same as before.
- **A book saved before `book correct` or `book leaf recalculate` ended holds marks, and v4.5.5-newedax.15 and earlier read it as a damaged book and do not load it.** A finished book has no mark.
- The eval tools (`tools/eval`) read the level without the marks.
- The search is not changed: the same values, node counts and lines as v4.5.5-newedax.15.
- The evaluation and its `eval.dat` files are not changed (those of v4.5.5-newedax.13, in the ZIPs of this release).

**Not checked**: a real large book, real Android devices, Windows ARM64.

[Tests and measurements](docs/validation/newedax.en.md#v455-heath16)

---

## v4.5.5-newedax.15

A fix of the exact search: at level 60, from 17 empties on, it rarely gave a wrong score (and move).

**Fixes**

- An exact search (a level whose depth is the number of empties, with no selectivity) could give a wrong score and move at the root (always a few discs wrong).
- In the data of the new `eval.dat` work, about 1 to 2 exact searches in 100,000 at 17 empties gave a wrong score. It happens with 1 or 4 threads, with any hash table size, on the same positions every time.
- The cause: an exact search first searches the position with selectivity (probcut), then exactly. Between those iterations, a node of the midgame search at the depth where the endgame search starts (`DEPTH_MIDGAME_TO_ENDGAME`, 15 empties) was stored in the hash table as exact, because its moves are searched by the endgame search.
- When that node has to pass, the search of the opponent stays in the midgame search at the same depth, where probcut can cut it. That score was stored as exact and taken by the exact iteration.
- Now such a node (a pass) is stored with the selectivity of the search.
- The same error is in the C versions v4.5.5-nikque.27 and v4.5.5-eval2.15, fixed in v4.5.5-nikque.28 and v4.5.5-eval2.16 ([the C release notes](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.md#v455-nikque28)).
- It is also in Edax 4.5.5 and 4.5.6 of Toshihiko Okuhara (edax-reversi-AVX), from which they come. Edax 4.6 and 4.7 of Richard Delorme do not have it.

**Compatibility**

- The scores of exact searches that were right do not change.
- The node counts change a little (0.01% more on 1,640 positions of 17 to 22 empties).
- Midgame searches (no endgame depth) are not changed.
- A book whose positions were searched exactly with an earlier version can hold such wrong scores (rarely).
- The rest of the search, the evaluation and its `eval.dat` files (those of v4.5.5-newedax.13, in the ZIPs of this release) and the book are not changed.

**Not checked**: real Android devices, Windows ARM64.

[Tests and measurements](docs/validation/newedax.en.md#v455-newedax15)

---

## v4.5.5-newedax.14

Edax can now think on the opponent's time under NBoard, and keep the hash tables from move to move. Both are new settings, off unless given.

**Changes**

- `-ponder on` (or `set ponder on`): the NBoard protocol thinks on the opponent's time too.
  - When the interface sends back with `move` the move of Edax's own `go`, Edax ponders: it guesses the opponent's move (from its last search) and searches the position after it.
  - When the opponent's move, given by `move` or by `set game` with the position after it, is the move guessed, Edax goes on with that search (the next `go` continues it). Another move stops it, and `go` searches as before.
  - `ping` does not stop the pondering. It stops the search of a `go` as before.
  - Without `-ponder on`, the NBoard protocol does not ponder, as before. `ponder` is on by default for the other protocols, so under NBoard it counts only when given.
- `-keep-hash on` (or `keep-hash on`; `set keep-hash on` under NBoard; off by default): a new board that continues the game keeps the hash tables.
  - A board continues the game when every square taken on the previous board is taken on the new one.
  - It works for `set game` of NBoard, `setboard` and the FEN of the Edax protocol, and the board of libedax.
  - Up to v4.5.5-newedax.13, the hash tables were cleaned at each new board (all their entries were dead). So an interface that sends the board of each move searched each move from empty tables, and a search on the person's turn (the pondering of nikque GUI) did not help the next move.
  - Under NBoard, the position pondered keeps the hash tables whatever this setting.
- The commands of an interface that plays under NBoard with `-ponder on -keep-hash on`:

```
nboard 1
set depth 60
ping 1                 → pong 1
set game <GGF>         the position, Edax to move
go                     → === <move> <score> <time>   (or, at the time limit, "ping 2": === at once, then pong 2)
move <move>            the move of ===: Edax ponders on the opponent's time
ping 3                 → pong 3   (it does not stop the pondering)
set game <GGF>         the position after the opponent's move (or: move <the opponent's move>)
go                     → goes on with the search pondered when the opponent played the move guessed
```

- New test `tests/nboard-ponder.py`: it checks the changes of this version with the standard error of `-info`.
- Manual: the settings `ponder` and `keep-hash` (4.6, 14) and the pondering under NBoard (12.1).

**Fixes**

- Under NBoard, a `ping` that comes just after a `go`, before its search starts, now stops the search too. Up to v4.5.5-newedax.13 that stop was lost, and the search went on to its own limit.

**Compatibility**

- Without `-ponder on` and `-keep-hash on`, the answers are those of v4.5.5-newedax.13. The only difference is the stop of a `ping` given just after a `go`.
- With `-keep-hash on`, a search uses what the searches of the previous moves found: its scores and node counts are not those of a search from empty tables.
- With `-ponder on` under NBoard, Edax uses its threads on the opponent's time. On the same CPUs as another engine, it takes time from that engine.
- The search, the evaluation, the `eval.dat` files (those of v4.5.5-newedax.13, in the ZIPs of this release) and the book are not changed.

**Not checked**: the strength with and without the pondering (no games were played for this release), NBoard GUI itself, real Android devices, Windows ARM64.

[Tests and measurements](docs/validation/newedax.en.md#v455-newedax14)

---

## v4.5.5-newedax.13

A new evaluation function: 64 feature slots, trained again (the default), and a 48-slot build. At the same time, the 64-slot build is +60.2 Elo against eval2 (v4.5.5-newedax.12).

**Changes**

- The release has two ZIPs.
  - `edax-4.5.5-newedax.13-64.zip`: the default build, 64 slots.
  - `edax-4.5.5-newedax.13-48.zip`: 48 slots, the 64 without 2 shapes; faster per node.
  - Each holds its own `eval.dat` (107,612,578 bytes).
- Evaluation (default build, `EVAL_SLOTS` 64): the 13 groups of Edax 4.5, 13 extra groups (`M X W C`, weights of 3^10 squares or 3^9), 15 shapes and the mobility weights in 64 slots.
- The shapes were chosen from 2026-11-05 to 11-13 by searches on the error of the evaluation and level 18/24 games (the shapes and their squares in the table that follows).
- The weights were trained again with `tools/eval/evsearch` (plies 2-53).
- The 48-slot build (`EVAL_SLOTS=48`): the same source and the same `eval.dat` format as the 64 slots, without the shapes `cxd10` and `edw10` (16 slots). Its weights were trained without them.
- The feature updates of the 48-slot build use 3 vectors of 256 bits in place of 4. It searches about 11 % more nodes in the same time, but its evaluation loses more: at the same time it is −12.1 Elo (−17.3 to −6.9) against the 64 slots.
- `EVAL_SLOTS` 48 no longer means eval2. The eval2 evaluation (46 patterns) can no longer be built.
- Probcut: `refit` is the default (`-probcut-model refit`, and `probcut-model = refit` in `bin/config.ini`).
- Search: a thread that waits polls before it sleeps, and the threads split from depth 9.
- AVX-512 builds read the weights of a ply from one table, one by one.
- Tools (`tools/eval`): `evshape`; `evsearch -mute`, `-method sd`; `w2dat2 -interp` and the extra groups; `selfplay3 -h`; refit in the search of `selfplay3` and `label`; `pcsigma`.
- Evaluation file: the 64-slot and the 48-slot builds give a warning when the file has no weights for their extra groups (the `eval.dat` of eval2 or of Edax 4.5).
- Tests (`tests/regression.cpp`): `evalslots` reads the table of AVX-512 builds, counts the extra weights of the file and checks the update on every slot.
- Tests (`tests/regression.cpp`): the cases of the files of form 1 (`evalform2`, `evalform2mob`, `evalbadconv`, `evalmobility`) make their file of form 1 from the 13 groups of Edax 4.5 of an `eval.dat` of form 2.

The shapes of the 64 slots (slots: the images used):

| shape | squares | slots |
|---|---|---|
| `c10` (Edax) | a5 a4 a3 a2 a1 b2 b1 c1 d1 e1 | 4 |
| `s100` (Edax) | b2 a1 b1 c1 d1 e1 f1 g1 h1 g2 | 4 |
| `m10` | b1 c1 d1 e1 f1 g1 c2 d2 e2 f2 | 4 |
| `mi10` | b2 c2 d2 e2 f2 g2 c3 d3 e3 f3 | 4 |
| `n10` | a1 b1 c1 f1 g1 h1 a2 c2 f2 h2 | 4 |
| `d64` | c1 e1 d2 f2 e3 g3 f4 h4 g5 h6 | 4 |
| `hm10` | g5 h5 g6 h6 e7 f7 h7 e8 f8 g8 | 4 |
| `l3w10` | c1 c2 b3 c3 c4 c5 b6 c6 c7 c8 | 4 |
| `db10` | d1 e2 d3 e3 f3 e4 f4 g4 f5 h5 | 4 |
| `ct10` | a1 b1 c1 g1 a2 b2 c2 a3 b3 a7 | 4 |
| `dx10` | b1 b2 c2 d3 e4 f5 g6 g7 h7 a8 | 4 |
| `d8c` | a1 h1 b2 c3 d4 e5 f6 g7 a8 h8 | 2 |
| `d3e10` | c1 b2 e2 a3 g4 b5 h6 d7 g7 f8 | 2 |
| `cxd10` (not in the 48 slots) | c1 d1 g1 h1 b2 d2 f2 g2 h2 f3 | 8 |
| `edw10` (not in the 48 slots) | g5 f6 e7 f7 g7 d8 e8 f8 g8 h8 | 8 |

<details>
<summary>The shapes on the board (15)</summary>

The shapes on the board (X the squares of the shape, . the other squares; in the first slot as in the table):

`c10` (Edax)

```
  a b c d e f g h
1 X X X X X . . .
2 X X . . . . . .
3 X . . . . . . .
4 X . . . . . . .
5 X . . . . . . .
6 . . . . . . . .
7 . . . . . . . .
8 . . . . . . . .
```

`s100` (Edax)

```
  a b c d e f g h
1 X X X X X X X X
2 . X . . . . X .
3 . . . . . . . .
4 . . . . . . . .
5 . . . . . . . .
6 . . . . . . . .
7 . . . . . . . .
8 . . . . . . . .
```

`m10`

```
  a b c d e f g h
1 . X X X X X X .
2 . . X X X X . .
3 . . . . . . . .
4 . . . . . . . .
5 . . . . . . . .
6 . . . . . . . .
7 . . . . . . . .
8 . . . . . . . .
```

`mi10`

```
  a b c d e f g h
1 . . . . . . . .
2 . X X X X X X .
3 . . X X X X . .
4 . . . . . . . .
5 . . . . . . . .
6 . . . . . . . .
7 . . . . . . . .
8 . . . . . . . .
```

`n10`

```
  a b c d e f g h
1 X X X . . X X X
2 X . X . . X . X
3 . . . . . . . .
4 . . . . . . . .
5 . . . . . . . .
6 . . . . . . . .
7 . . . . . . . .
8 . . . . . . . .
```

`d64`

```
  a b c d e f g h
1 . . X . X . . .
2 . . . X . X . .
3 . . . . X . X .
4 . . . . . X . X
5 . . . . . . X .
6 . . . . . . . X
7 . . . . . . . .
8 . . . . . . . .
```

`hm10`

```
  a b c d e f g h
1 . . . . . . . .
2 . . . . . . . .
3 . . . . . . . .
4 . . . . . . . .
5 . . . . . . X X
6 . . . . . . X X
7 . . . . X X . X
8 . . . . X X X .
```

`l3w10`

```
  a b c d e f g h
1 . . X . . . . .
2 . . X . . . . .
3 . X X . . . . .
4 . . X . . . . .
5 . . X . . . . .
6 . X X . . . . .
7 . . X . . . . .
8 . . X . . . . .
```

`db10`

```
  a b c d e f g h
1 . . . X . . . .
2 . . . . X . . .
3 . . . X X X . .
4 . . . . X X X .
5 . . . . . X . X
6 . . . . . . . .
7 . . . . . . . .
8 . . . . . . . .
```

`ct10`

```
  a b c d e f g h
1 X X X . . . X .
2 X X X . . . . .
3 X X . . . . . .
4 . . . . . . . .
5 . . . . . . . .
6 . . . . . . . .
7 X . . . . . . .
8 . . . . . . . .
```

`dx10`

```
  a b c d e f g h
1 . X . . . . . .
2 . X X . . . . .
3 . . . X . . . .
4 . . . . X . . .
5 . . . . . X . .
6 . . . . . . X .
7 . . . . . . X X
8 X . . . . . . .
```

`d8c`

```
  a b c d e f g h
1 X . . . . . . X
2 . X . . . . . .
3 . . X . . . . .
4 . . . X . . . .
5 . . . . X . . .
6 . . . . . X . .
7 . . . . . . X .
8 X . . . . . . X
```

`d3e10`

```
  a b c d e f g h
1 . . X . . . . .
2 . X . . X . . .
3 X . . . . . . .
4 . . . . . . X .
5 . X . . . . . .
6 . . . . . . . X
7 . . . X . . X .
8 . . . . . X . .
```

`cxd10` (not in the 48 slots)

```
  a b c d e f g h
1 . . X X . . X X
2 . X . X . X X X
3 . . . . . X . .
4 . . . . . . . .
5 . . . . . . . .
6 . . . . . . . .
7 . . . . . . . .
8 . . . . . . . .
```

`edw10` (not in the 48 slots)

```
  a b c d e f g h
1 . . . . . . . .
2 . . . . . . . .
3 . . . . . . . .
4 . . . . . . . .
5 . . . . . . X .
6 . . . . . X . .
7 . . . . X X X .
8 . . . X X X X X
```

</details>

**Strength**

Games of the 64-slot build (refit, its PGO build), without a book, from random openings, each played with both colours. The games against eval2, the weights of 11/05 and Edax were played with one thread on each side (`-n 1`); the time is that of each side. The thread count of the games against the 48-slot build is not in the records. The Elo is that of the new build; the brackets are 95% intervals.

| against | condition | games | Elo |
|---|---|---|---|
| eval2 (v4.5.5-newedax.12, standard) | 30 s a game a side | 1,102 | +60.2 (+47.9 to +72.6) |
| the weights of 11/05 (64 slots, refit) | 30 s a game a side | 2,204 | +11.2 (+3.7 to +18.7) |
| Edax with the original `eval.dat` | level 18 both | 4,378 | +77.5 (+69.9 to +85.2) |
| Egaroucid (tournament branch `dev-lazysmp-ggs-b`) | 120 s a game, 8 threads | 180 | +23.2 (+3.0 to +43.6) |
| the 48-slot build (refit) | 30 s a game a side | 4,378 | +12.1 (+6.9 to +17.3) |

- Probcut: `refit` was measured against `standard` on these weights: `standard` is −1.0 Elo (−5.9 to +3.8, 4,378 games) for the 64 slots, −7.7 (−14.8 to −0.7, 2,204 games) for the 48 slots.

**Compatibility**

- The search results change (the evaluation and the default probcut model). `-probcut-model standard` gives the former probcut model.
- The book file format does not change. Books learned with an earlier version are read, but their values were given by the former evaluation.
- `eval.dat`: the file of each build is in its ZIP (`bin/data/eval.dat`), not in git. A build of the repository needs it too.
- The `eval.dat` of eval2 that stays in `bin/data` of the repository and the one of Edax 4.5 cannot be used by the 64 and the 48 slots (they are read with a warning, but the evaluation is then wrong).
- The 64-slot and the 48-slot `eval.dat` have the same format, but each is for its own build.
- libedax: the same API as v4.5.5-newedax.12. A program that ships the library must ship the `eval.dat` of the same build.

**Not checked**: real Android devices, Windows ARM64, the speed of the books, builds other than MSVC on Windows and the NDK on Android.

[Tests and measurements](docs/validation/newedax.en.md#v455-newedax13)

---

## v4.5.5-newedax.12

As in the C version, a book file that another program holds when Edax starts is no longer taken for a damaged one and replaced. Other bugs found in a review are fixed, and the code was rearranged without changing what it does.

**Changes**

- The changes of the C version v4.5.5-nikque.27 are included ([the C release notes](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.md#v455-nikque27)).
  - A book file that cannot be opened at the start is tried again after about 2 seconds. If it still cannot be opened, it is never saved to (no `.damaged` file either).
  - `edax_book_file_status` was added to libedax.
  - The code was rearranged without changing what it does.
- The eval tools (`tools/eval`): the errors of opening a file and of memory are given by the same functions (`xfopen`, `xmalloc`, `xcalloc`, `die`), with the same messages and exit codes.
- Tests: new checks in `tests/libedax_test.c`, `tests/book-analyze.cpp` and `tests/regression.cpp`.
- Manual: 4.1, 6.6, 7.3, 8.4, 8.5, 12.2, 12.4 and 13.1 (the same text as the C version, but the number of checks in 12.2).

**Fixes**

- The same fixes as the C version v4.5.5-nikque.27: the marks of the book files that could not be loaded, `book import`, `book merge`, the `export` of the `book deviate` family, `book moveinfo`, `book reducecount`, the time `5:`, the place of `config.ini`, the table and the statistics of `weval`, libedax and others.
- The eval tools: when the name of a file made from the name of a folder did not fit in its array, the name was written beyond the array, and `sprepair`, `bookdump` and `w2dat2` crashed or went on with a damaged name. Now the tool ends with `file name too long` and the exit code 1 (22 places in 10 tools).
- The eval tools: `leafgen` and `w2dat2` end with `out of memory` when an allocation fails.
- The case `evalmobility` of `tests/regression.cpp` always failed with the `eval.dat` of newedax, which holds mobility weights. It now passes (its file without mobility weights is written from the weights of the 61 plies alone).

**Compatibility**

- The search results, `eval.dat` and the book file format do not change.
- The answers differ from v4.5.5-newedax.11 only in the cases fixed in this version. The error messages of the book give a line of the source, which changed.
- libedax: when the book file of the settings could not be opened at the start, no save replaces it (`edax_book_save_checked` and `edax_book_keep_unread_file` give 0). A program that saves the book itself should look at `edax_book_file_status` after the initialization.
- The eval tools give the same outputs as those of newedax.11 for the same inputs, but for a name too long.

**Not checked**: the time and the memory on a real book, real Android devices, what the eval tools do without memory (read only).

[Tests and measurements](docs/validation/newedax.en.md#v455-newedax12)

---

## v4.5.5-newedax.11

As in the C version, `book deviate`, `book deviate2` and `book deviate3` get `export`: it writes the lines they walk to a text file, without expanding the book.

**Changes**

- The changes of the C version v4.5.5-nikque.26 are included ([the C release notes](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.md#v455-nikque26)).
  - `export <file> [leaf|ends|all]` was added.
  - `edax_book_deviate_export` was added to libedax.
  - The mark `@cancel id=N state=none export=1` was added.
- Manual: 7.3 and 12.4 (the same text as the C version).

**Compatibility**

- The commands without `export` are the same as in v4.5.5-newedax.10.
- The search results, `eval.dat` and the book file format do not change.

**Not checked**: the time and the memory on a real book, real Android devices.

[Tests and measurements](docs/validation/newedax.en.md#v455-newedax11)

---

## v4.5.5-newedax.10

As in the C version, an umigame number marked `exact=0` is the same whatever the build.

**Changes**

- The changes of the C version v4.5.5-nikque.25 are included ([the C release notes](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.md#v455-nikque25)).
- Manual: 12.4 says so (the same text as the C version).

**Compatibility**

- A number marked `exact=0` may differ from v4.5.5-newedax.9 (both are upper bounds). The other answers are the same.
- The search results, `eval.dat` and the book file format do not change.

**Not checked**: the speed on a real book, the behaviour in the GUI, real Android devices.

[Tests and measurements](docs/validation/newedax.en.md#v455-newedax10)

---

## v4.5.5-newedax.9

As in the C version, the umigame numbers and the numbers of positions of the candidate moves of the GUI, `book levels` and `book missing` are taken over from the answers of the positions before, so that the same counts are not made again.

**Changes**

- The changes of the C version v4.5.5-nikque.24 are included ([the C release notes](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.md#v455-nikque24)).
  - Umigame numbers: when the graph of the position after a move is counted, the values of the positions 1 and 2 moves further in it are kept. Their graphs are not built again when they become the current position.
  - Numbers of positions (`part=count`) and `book levels`: the answer of each position is kept until the book changes.
  - `book missing`: when the kept accuracy stages already tell that the range asked finds nothing, it answers 0 lines without walking.
- Manual: 12.4 describes these three changes (the same text as the C version).

**Compatibility**

- The answers of `book moveinfo`, `book levels` and `book missing` are the same as in v4.5.5-newedax.8.
- But an umigame number marked `exact=0` when counted from its position alone may become the exact value kept from the position before (not marked, smaller or equal).
- The search results, `eval.dat` and the book file format do not change.

**Not checked**: the speed on a real book, the behaviour in the GUI, real Android devices.

[Tests and measurements](docs/validation/newedax.en.md#v455-newedax9)

---

## v4.5.5-newedax.8

As in the C version, the accuracy of the candidate moves of the GUI (`stage=` of `book moveinfo`) is taken over from the answers of the positions before and after, so that less is walked. A book command typed without `book` gets a hint on how to type it. The manual and the documents were rewritten.

**Changes**

- The changes of the C version v4.5.5-nikque.23 are included ([the C release notes](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.md#v455-nikque23)).
  - What the kept stage of the current position and the kept stages of the next positions of the position after a move already tell is not walked again. The answers (`stage=`, `missing=`) are the same as in v4.5.5-newedax.7.
  - A book command typed at the top level (`leaf expand 30 30`, `fill2 2`, …) adds `(book command: type "book leaf expand 30 30" or "b leaf expand 30 30")` to the warning.
  - Names that are also top-level commands, settings or `base` commands get no hint.
- Manual: the same review as the C version.
  - The wording was reviewed.
  - Every book command is written in the form `book …`.
  - The time of a save in 9.4 is now 18 to 28 seconds.
  - The note on `refit` in chapter 4 links to nikque.5 in the C release notes.
  - 12.4 and 13.2 describe the two changes of this version.
- Documents: the README and the release notes were rewritten, the test and measurement records moved to `docs/validation/`, and `README-NIKQUE.*` was removed.
- The release ZIP holds `README.md` instead of `README-NIKQUE.*`. The first comment lines of `config.ini` point to chapter 4 of the manual.

**Compatibility**

- The answers of `book moveinfo` are the same as in v4.5.5-newedax.7.
- The warning line of an unknown command gets a hint at its end only for the names of book commands (its first part is the same).
- The search results, `eval.dat` and the book file format do not change.

**Not checked**: the speed on a real book, the behaviour in the GUI, real Android devices.

[Tests and measurements](docs/validation/newedax.en.md#v455-newedax8)

---

## v4.5.5-newedax.7

`book reduce` and `book reducecount` are now as fast and as light on memory as in the C version.

**Changes**

- The changes of the C version v4.5.5-nikque.22 are included ([the C release notes](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.md#v455-nikque22)).
  - `book reducecount` is faster, and a `book reduce` right after it reuses what was counted. After the cut, the memory no longer used is freed at once.
  - New settings `book-reduce-index`, `book-reduce-reuse`, `book-reduce-shrink` and `book-reduce-walk` were added (all of them work as they are by default).
- The answers of the commands are those of the C version v4.5.5-nikque.22.

**Compatibility**

- The positions kept and what each position holds are those of v4.5.5-newedax.6.
- The results of the search, `eval.dat` and the book file format are not changed.

**Not checked**: the speed in newedax (the speed of `book reducecount` on the real book of 29 GB was measured only with the C version), real Android devices.

[Tests and measurements](docs/validation/newedax.en.md#v455-newedax7)

---

## v4.5.5-newedax.6

`book reduce` and `book reducecount` get the same fixes and the same new way of working as the C version.

**Changes**

- The changes of the C version v4.5.5-nikque.21 are included ([the C release notes](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.md#v455-nikque21)).
  - `book reducecount` now runs in parallel with the `n-tasks` threads.
  - `book reducecount` did not end with a damaged book, and positions were removed even with wrong input. Both were fixed (now an error or a warning is shown and the book is not changed).
- The answers of the commands are those of the C version v4.5.5-nikque.21.

**Compatibility**

- With a sound book, the positions kept and the book saved after `book reduce` are those of v4.5.5-newedax.5.
- The results of the search, `eval.dat` and the book file format are not changed.

**Not checked**: the real book of 29 GB, real Android devices.

[Tests and measurements](docs/validation/newedax.en.md#v455-newedax6)

---

## v4.5.5-newedax.5

The command `book reduce`, which makes the book smaller, and `book reducecount`, which counts first, were added.

**Changes**

- The changes of the C version v4.5.5-nikque.20 are included ([the C release notes](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.md#v455-nikque20)).
  - `book reduce X Y` keeps only the positions in the range that `book deviate3 X Y` walks, and removes the others. `book reducecount` counts how many are kept and how many are removed (the book is not changed).
- The commands and answers are those of the C versions v4.5.5-nikque.20 and v4.5.5-eval2.8.
- How to use them: [Manual 8.4](manual/en/08-book-maintenance.md), [12.4](manual/en/12-integration.md)

**Compatibility**: the results of the search, `eval.dat` and the book file format are those of v4.5.5-newedax.4.

**Not checked**: the speed of the new commands, the real book of 29 GB, real Android devices.

[Tests and measurements](docs/validation/newedax.en.md#v455-newedax5)

---

## v4.5.5-newedax.4

Book analysis commands for GUI programs were added.

**Changes**

- The changes of the C version v4.5.5-nikque.19 are included ([the C release notes](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.md#v455-nikque19)).
  - New commands: `book moveinfo`, `book missing`, `book levels`, `book bestlines`, `book findline`, and `stop id=N`.
- The commands and answers are those of the C versions v4.5.5-nikque.19 and v4.5.5-eval2.7.
- None of the new commands is slower than in the C version (a book of 19.14 million positions, 1 thread: counting the positions from the first move takes 5.03 s against 5.92 s for the C version). The working memory of the book alone is 880 MB against 1,235 MB for the C version.
- The Windows executables are built as in v4.5.5-newedax.3 (`wEdax-x86-64-v4.exe` without PGO, the other 4 with PGO).
- How to use them: [Manual 12.4](manual/en/12-integration.md)

**Compatibility**: the results of the search, `eval.dat` and the book file format are those of v4.5.5-newedax.3.

**Not checked**: the real book of 29 GB, real Android devices.

[Tests and measurements](docs/validation/newedax.en.md#v455-newedax4)

---

## v4.5.5-newedax.3

The way the Windows executables are built is changed, so that their speed with 1 thread matches the C version. The first full release that is not a pre-release.

**Changes**

- Of the Windows executables, `wEdax-x86-64-v4.exe` is now built without PGO (optimization that uses records of runs), and the other 4 with PGO (`tools/port/build-release.cmd`). In v4.5.5-newedax.2, none used PGO.
- `wEdax-x86-64-v4.exe` is faster without PGO, so its machine code stays that of v4.5.5-newedax.2. The 3 DLLs and Android are without PGO too, and their machine code is not changed.
- Speed (ratio against the C version v4.5.5-eval2.6): in the midgame with 1 thread, `wEdax-x86-64-v4.exe` is about 3 to 4% faster than the C version (0.961 to 0.969). `wEdax-x86-64.exe`, `wEdax-x86-sse.exe` and `wEdax-x86.exe` are about as fast as the C version or faster.
- **`wEdax-x86-64-v3.exe` is still about 1% slower than the C version** (the midgame with 1 thread; `book learn` and the `fill` commands with 32 threads). On AVX-512 CPUs, use `wEdax-x86-64-v4.exe`.
- The peak memory is the same as the C version or less (`book import` 614 → 545 MiB). When loading a book of 34.73 million positions made by the C version, the peak memory is 3,243 → 1,667 MiB.
- Of the eval2 tools, `w2dat2` and `label` are about 0.7% slower than the C tools. The others are as fast or faster.
- Record of the measurements: `docs/port/MEASURE-2.md`.

**Compatibility**

- The results of the search are those of v4.5.5-newedax.2 (and of v4.5.5-eval2.6).
- `eval.dat` and the book file format are not changed.

**Notes**: with PGO, the machine code changes a little with each build. The released `-v3`, `wEdax-x86-sse.exe` and `wEdax-x86.exe` are the very files that were compared with the C version again.

**Not checked**: why `wEdax-x86-64-v3.exe`, `w2dat2` and `label` are slower, real Android devices.

[Tests and measurements](docs/validation/newedax.en.md#v455-newedax3)

---

## v4.5.5-newedax.2

The errors found by the cross-check of v4.5.5-newedax.1 were fixed. Some executables are still slower than the C version with 1 thread, so this is still a pre-release.

**Changes**

- The executables and libraries no longer hold the folder in which they were built (the same fix as the C version v4.5.5-nikque.18; [the C release notes](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.md#v455-nikque18)). Error messages now show only the file name of the source.
- `vcomp140.dll` (the MSVC OpenMP runtime library) is now in `tools/eval` of the ZIP. 7 tools use it.
- The tool scripts take the input folders as arguments (`--ev`, `--book40`, `-Inputs`; no default value).
- The Windows executables are without PGO, as in v4.5.5-newedax.1.
- Speed (`wEdax-x86-64-v4.exe` with 32 threads): the same as v4.5.5-newedax.1, and as fast as the C version v4.5.5-eval2.6 or faster (0.959 to 1.001 times the time for 7 book commands). The peak memory is the same or less.

**Fixes** (the eval2 tools and `-eval-convert`)

- `bookdump` and `bookscan`: with a book that had a position of fewer than 4 discs, the move number was negative (the C tools have the same error). Such books are now refused.
- When the same scalar was written twice in the tool settings, it was counted twice. When a pattern had the same square twice, the symmetric mapping was broken. Both are now refused.
- In the 5-byte numbers of format 2, the upper 4 bits of the 5th byte were not checked (2^32−1 is read, 2^32 is refused).
- `build-evtools.ps1` returned the exit code 0 even when some tools failed to build. It now returns 1.
- `-eval-convert`: it converted format 1 files with fewer than 54 move numbers, and made files that neither format could read. In format 2, the number of move numbers × weights overflowed when it did not fit in 32 bits. Both are now refused.

**Compatibility**: the results of the search are those of v4.5.5-newedax.1 (and of v4.5.5-eval2.6).

**Notes**: with 1 thread, the 4 executables other than `wEdax-x86-64-v4.exe` are slower than the C version (a PGO build) (up to 11.6% in the midgame, `wEdax-x86-sse.exe`). This is fixed in v4.5.5-newedax.3.

**Not checked**: the strength of the default build (eval2), builds other than MSVC on Windows and the NDK on Android, real Android devices, AddressSanitizer and ThreadSanitizer, real books of billions of positions, a second reading by another person.

[Tests and measurements](docs/validation/newedax.en.md#v455-newedax2)

---

## v4.5.5-newedax.1

The first version of new edax. The C version v4.5.5-nikque.17 is ported to C++23, and the evaluation function of the eval2 line (v4.5.5-eval2.5) is the default. The sources are not yet checked by another person, and the speed of the default build is not yet measured, so this is a pre-release.

**Changes**

- In the default build, the build option `EVAL_MOBILITY` is 1 and `bin/data/eval.dat` is the eval2 one.
- The commands and options are those of the C version.
- Format 2 of the book and evaluation files (`NEDXBOOK`, `NEDXEVAL`) was added. Books are about 33% smaller and get a CRC-32C. Format 1 is read too.
- Options that change the format:

| Option | What it does |
|---|---|
| `-book-format` | Chooses the book file format |
| `-book-convert` | Converts the book format (both ways) |
| `-eval-convert` | Converts the evaluation file format (both ways) |

- A book position now takes 40 bytes in memory (48 bytes in the C version). Loading a book of 6.49 million positions takes 11 to 45% less memory.
- New build options: `EVAL_MOBILITY=0` (the mobility weights are not read), `EVAL_SLOTS=64` (64 slots; a base for a stronger evaluation function; the default is 48).
- `tools/eval`: the C++ version of the learning tools of the eval2 evaluation function (14 tools; the ZIP has only the x64 AVX-512 ones).
- Speed: compared with the C version v4.5.5-nikque.17 built the same way (without PGO), no case slower than the C version was found (`docs/port/MEASURE-1.md`).
- Release builds (MSVC 2022 `/O2 /GL`, **without PGO**; Android with NDK r27d):
  - Windows: `wEdax-x86-64-v4.exe`, `wEdax-x86-64-v3.exe`, `wEdax-x86-64.exe`, `wEdax-x86-sse.exe`, `wEdax-x86.exe`, `libedax-x64-v4.dll`, `libedax-x64-v3.dll`, `libedax-x64.dll`.
  - Android: `aEdax-arm64-v8a`, `aEdax-armeabi-v7a`, `libedax-arm64-v8a.so`, `libedax-armeabi-v7a.so`.
  - There are no executables for Linux, macOS or Windows ARM64 (to build from the sources, see "Build" in the README).
  - PGO is not used because the `book deviate` commands were about 1.5 to 2% faster without PGO (the same in the C version). In other cases, PGO and no PGO were not compared directly.

**Compatibility**

- The results of the search are those of the C version: those of v4.5.5-eval2.5 with the eval2 `eval.dat`, and those of v4.5.5-nikque.17 with the Edax 4.5 `eval.dat`.
- Format 1 books and `eval.dat` are read as they are.

**Not checked**: the speed and strength of the default build (eval2), builds other than MSVC on Windows and the NDK on Android, real Android devices, tests with builds that detect memory errors, real books of billions of positions, a check of the sources by another person.

[Tests and measurements](docs/validation/newedax.en.md#v455-newedax1)
