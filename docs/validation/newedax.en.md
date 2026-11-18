# Heath (formerly newedax): tests and measurements

Tests and measurements done for each version, moved without changes from the former README-NIKQUE.en.md.

For what changed, see the [release notes](../../RELEASE-NOTES.md). "Above" and "below" in the text refer to positions in the former document.


## v4.5.5-heath.16

Checks:

- `tests/book-resume.py` (new, 41 checks, one logical CPU): `book correct` with 1 and 2 threads (marked positions with a wrong score are not searched again, the marks go, a third run finds the wrong scores), `book deepen`, `book leaf recalculate3`, `book learn` (also a run stopped as soon as its progress file was written, then a new run on the book saved to `.store`), the marks kept by a book saved in form 2 and read again, and v4.5.5-newedax.15 reading a book with marks as a damaged book.
- The 14 book suites of the C version (1 thread, without `book-reproducible`; the 64-slot build and `eval.dat`), with a build of this source and with v4.5.5-newedax.15: the same files except the save date in 13 suites; S3 (`book learn` with the random seed of the clock) differs from run to run, and with `-book-seed 1` both give the same books.
- `tests/regression.cpp`: the 54 cases pass (64 slots, AVX-512).
- The release programs (64 and 48 slots, with the `eval.dat` of their build, that of v4.5.5-newedax.13): in each build, the 5 Windows programs give at level 12 on 40 midgame positions the same values, node counts and lines as v4.5.5-newedax.15; fforum-20-39 (1 thread) the same scores and moves, and at `-l 60` 0 wrong move and 0 wrong score; the 1,205 positions of the fix of v4.5.5-newedax.15 at `-l 60`: 0 wrong score; `tests/cassio-eof.py` (7 cases) and `tests/nboard-ponder.py` pass; the 3 DLLs of each build pass the 281 API checks.
- The eval tools: 16 programs built with `tools/eval/build-evtools.ps1` (x64 AVX-512, Boost 1.92).
- Not checked: a real book, real Android devices, Windows ARM64.

### Moved from the release notes

**Checked**: `tests/book-resume.py` (41 checks, also in form 2); the 14 book suites of the C version give the same books as v4.5.5-newedax.15 (except the save date; with `-book-seed 1` for the suite that learns with a random seed from the clock); `tests/regression.cpp` (54 cases); the release programs of both builds give the same values, node counts and lines as v4.5.5-newedax.15.

## v4.5.5-newedax.15

Checks:

- The cause, found on the positions of the new `eval.dat` work whose exact scores disagreed between a parent and its children: with the score of each search stored in the hash table recorded and searched again exactly afterwards, every wrong score came from one entry, of depth 15, selectivity 0 and score = beta, stored by a node that passed. With probcut, the hash table or the hack of that entry turned off, the wrong scores were gone; with ETC, the stability cutoff or the PV extension turned off, they stayed.
- 1,205 positions (17 to 20 empties, most of them 17; the exact scores of an independent solver without hash table): 504 of them were wrong before the fix with the eval of that work, 285 with the `eval.dat` of the 64 slots; 0 after it, with 1 and 4 threads and hash tables of 2^20 and 2^24. 1,936 positions of 16 empties: 5 wrong before, 0 after. 1,640 positions of 17 to 22 empties taken at random: the same scores before and after, 0.011% more nodes.
- The same 1,205 positions with other engines (1 thread, hash table 2^20, the `eval.dat` of Edax 4.5.5): Edax 4.5.5 and 4.5.6 of Toshihiko Okuhara (edax-reversi-AVX, and its master of 2026-11-03) 267 wrong, 0 with this fix; Edax 4.6 and 4.7 of Richard Delorme 0 (their hack stores a node as exact only when its depth is more than its empties, which does not happen).
- The release programs (64 and 48 slots, with the `eval.dat` of their build, that of v4.5.5-newedax.13): in each build, the 5 Windows programs give at level 12 on 40 midgame positions the same values, node counts and lines as v4.5.5-newedax.14; on fforum-20-39 (1 thread, default level) the same scores and moves (77,808,331 nodes with the 64 slots, as v4.5.5-newedax.14; 73,019,237 with the 48 slots, 33 more), 0 wrong moves and scores with `-l 60`, and 0 wrong scores on the 1,205 positions above with `-l 60`. The 7 Cassio cases pass with the 10 programs; the 6 DLLs pass 281 API checks with 0 failures; `tests/nboard-ponder.py` passes (57 checks in each build). Android (NDK r27d): no error, the same 8 warnings as v4.5.5-newedax.14.
- `tests/regression.cpp`: the 54 cases pass with the 64 and the 48 slots (AVX-512).
- The eval tools: 16 programs built with `tools/eval/build-evtools.ps1` (x64 AVX-512, Boost 1.92) without an error.
- Not checked: real Android devices, Windows ARM64.

### Moved from the release notes

**Checked**: 1,205 positions whose exact scores were wrong (most of them with 17 empties; the scores of an independent solver): all right now, with 1 and 4 threads and hash tables of 2^20 and 2^24; the release programs below.

## v4.5.5-newedax.14

Checks:

- The release programs (64 and 48 slots, with the `eval.dat` of their build, which is that of v4.5.5-newedax.13): in each build, the 5 Windows programs give at level 12 on 40 midgame positions (no option) the same values, node counts and lines as the engine with which the weights were measured (the same check as for v4.5.5-newedax.13); on fforum-20-39 (1 thread, default level) the same answers and node counts as that engine (77,808,331 nodes with the 64 slots, 73,019,204 with the 48 slots, as v4.5.5-newedax.13), and 0 wrong moves and 0 wrong scores with `-l 60` (exact). The 7 Cassio cases pass with the 10 programs; the 6 DLLs pass 281 API checks with 0 failures. Android (NDK r27d): no error; 8 warnings, all the same one (`-Wdeprecated-volatile`: the `++` and `--` of the new `volatile int` counter of the NBoard `go` commands, which is only read and written under a lock; the generated code does the same).
- `tests/nboard-ponder.py` (2 threads), with the `wEdax-x86-64-v4.exe` of each build (57 checks with the 64 slots, 55 with the 48: 2 depend on the moves the build plays): after the move of its own `go` sent back with `move`, Edax ponders (its process uses the CPU) under `-ponder on`; `ping` does not stop the pondering; the opponent's move guessed, given by `set game` or by `move`, goes on with the search pondered (`[switch from pondering to thinking]` of `-info`), another move stops it; a `go` followed at once by a `ping` is answered at once (0.00 to 0.23 s); `set game` of the same game keeps the hash tables under `-keep-hash on`, of a new game cleans them; `set ponder off` stops the pondering, `set ponder on` and `set keep-hash on` switch them on; with `-ponder off` and with no option, no pondering and `set game` cleans the hash tables; with no option, at depth 12 with 1 thread, the moves and scores of 14 positions are those of the `wEdax-x86-64-v4.exe` of v4.5.5-newedax.13.
- The Edax protocol, as nikque GUI uses it (`setboard`, `level`, `go`; `setboard` and `hint 60` on the person's turn, then `stop`): with `keep-hash on`, the hash tables are not cleaned by the `setboard` of the next positions (6 cleanings without it, 0 with it). An engine without the setting (v4.5.5-newedax.13) answers `keep-hash on` with the warning `Unknown command` and goes on.
- `tests/regression.cpp`: the 54 cases pass with the 64 and the 48 slots (AVX-512).
- The eval tools: 16 programs built with `tools/eval/build-evtools.ps1` (x64 AVX-512, Boost 1.92) without an error.
- Not checked: the strength with and without the pondering (no games), NBoard GUI itself, real Android devices, Windows ARM64.

### Moved from the release notes

- Tests: `tests/nboard-ponder.py` (57 checks of the above, with the standard error of `-info`).

**Checked**: see the tests below: the checks of `tests/nboard-ponder.py`, and the release programs give the same results and node counts as those of v4.5.5-newedax.13.

## v4.5.5-newedax.13

Checks:

- The weights and the engine: in each build (64 and 48 slots), the 5 Windows release programs give at level 12 on 40 midgame positions, with no option (the default probcut model `refit`), the same values, node counts and lines as the engine with which the weights were measured (2026-11-13, `-probcut-model refit`). Before the release builds, the same check passed on builds with AVX-512, AVX2, no SIMD flag and `/fp:fast` (the cl options of the release), with `-probcut-model refit` and with no option.
- fforum-20-39 (1 thread): at the default level, the 10 Windows programs give the same answers and node counts as the engine with which the weights were measured (77,808,331 nodes with the 64 slots, 73,019,204 with the 48 slots; they differ from those of v4.5.5-newedax.12, as the evaluation changed). There, the position 18 is searched at 22@98% (not exactly) and gives h3 (−22) in place of g2 (−20), with both builds, as that engine does. With `-l 60` (exact), the 10 programs give 0 wrong moves and 0 wrong scores. The 7 Cassio cases pass with the 10 programs. The 6 DLLs pass 281 API checks with 0 failures with the `eval.dat` of their build. Android (NDK r27d): no warning, no error.
- `tests/regression.cpp`: the 54 cases pass with the 64 and the 48 slots, built with AVX-512 and for 32-bit x86 without SSE (as `wEdax-x86.exe`). The cases of the files of form 1 now make their file from the 13 groups of Edax 4.5 of the `eval.dat` of form 2, and `evalslots` counts the extra weights of the file (with the `eval.dat` of this version, 5 cases failed before these changes of the tests).
- Fixed before the release: the 48-slot `wEdax-x86.exe` (32-bit, no SSE) stopped in its PGO run: without SIMD, the 48 slots used the update of Edax 4.5 square by square, which does not know the shapes of this version. It now uses the update of the 64 slots (the regression cases above check it).
- The eval tools: 16 programs built with `tools/eval/build-evtools.ps1` (x64 AVX-512, Boost 1.92) without an error.
- Strength: see the table of the release notes. The games were played with builds of the same source (the PGO build of the 64 slots, `refit`), from random openings, each played with both colours, without a book; the weights of the release are those of the measures.
- Not checked: real Android devices, Windows ARM64, the speed of the books, the time and memory on a real book.

## v4.5.5-newedax.12

Checks:

- Before and after the changes (the tests of the C version): on test books with 1 and 4 threads, the book commands (76 cases) write the same files and give the same answers, the analysis commands (20,046) give the same answers, and 25 cases of the other commands give the same output (but the time and the line numbers of the book source). The new checks of the bugs fail with the code before the fixes, but a few that cannot hit the bug on purpose (checked with test copies of the code). The gcc 11.4 build (WSL) gives as many warnings as before, and its library passes 251 checks (all but the checks for Windows only) with 0 failures.
- With the released `wEdax-x86-64-v3.exe`: the `book reduce` tests (500 cases), the 14 tests of damaged books and wrong words and the earlier tests of the analysis commands (3 small books, the real `book deviate` family, 40,000 random graphs of positions, the background thread, the lines of `book bestlines` and `book findline`) find nothing wrong. The 19 unit tests of the analyses (`tests/book-analyze.cpp`) pass (the 16 earlier ones with the numbers of the previous version). The tests of the precision stages (502,470 moves of 200 random books) and of the hint (24 cases) find nothing wrong.
- The answers of 10 commands on the same positions (6,880 answers) are all the same as those of the C version v4.5.5-nikque.27 (its released files) (except the time).
- Search: the 5 Windows release programs solve fforum-20-39 with 1 thread with the same results and node counts as the same check of v4.5.5-newedax.11: 78,733,223 with the `eval.dat` of eval2, 77,639,567 with that of Edax 4.5; the 7 Cassio cases pass. The 3 DLLs pass 281 API checks with 0 failures with both `eval.dat`. Android (NDK r27d): no warning.
- The 14 eval tools: built with `tools/eval/build-evtools.ps1` (x64 AVX-512) without an error or a warning. The C tools and their records are not at hand, so they were compared with the tools of newedax.11 (built from its source, and those of its ZIP) on the same inputs: in 26 cases of `tools/eval/compare_c.py`, the files written, the standard output (but the times) and the standard error are the same. `tools/eval/test_evtools.py`: 25 tests pass, and the one that needs an AVX2 build is skipped (the same with the tools of newedax.11). With a folder name of 1,100 characters, `sprepair`, `bookdump` and `w2dat2` end with `file name too long` (the tools of newedax.11 all crashed (the stack check, `0xC0000409`)). No real book was used: the inputs were made from a synthetic book, test books and self-play records.
- The search, the evaluation, the book learning and the file format are not changed. The node counts above check that the search is the same.
- Size (against v4.5.5-newedax.11): the 5 Windows programs −1,536 to +1,024 bytes, the 3 DLLs +1,024 to +2,560 bytes, the Android programs −808 and +352 and libraries +424 and +1,112 bytes (the fixes and the rearranged code); 11 eval tools have the same size, `leafgen` +1,024, `selfplay3` and `sprepair` +512 bytes.
- Speed: not measured.

### Moved from the release notes

- Tests: `tests/libedax_test.c` has 281 checks (61 more, 28 of them on Windows only), `tests/book-analyze.cpp` 3 more tests, `tests/regression.cpp` the new cases of the C version. The case `evalmobility` of `tests/regression.cpp` passes with the `eval.dat` of newedax, which holds mobility weights (its file without mobility weights is written from the weights of the 61 plies alone; before, it always failed with that `eval.dat`).

**Checked**: as for the C version, before and after the changes, on test books with 1 and 4 threads, the book commands (76 cases) write the same files and give the same answers, the analysis commands (20,046) give the same answers, and 25 cases of the other commands give the same output (but the time and the line numbers of the book source). The 281 checks of the test of libedax and the unit tests of the analyses pass; the gcc 11 build (WSL) gives the same warnings as before, and its library passes 251 checks (all but the checks for Windows only). 6,880 answers are the same as those of the C version v4.5.5-nikque.27. The eval tools give the same outputs as those of newedax.11 for the same inputs in 26 cases (both those built from its source and those of its ZIP; no real book among the inputs).

## v4.5.5-newedax.11

Checks:

- The test of the export: the lines written are compared with sets computed apart in Python (the positions with a leaf to expand, the ends, all the positions of the lines): on 41 positions of 3 test books, the 4,545 cases (3 commands, 5 ranges, 3 kinds) are all the same; every line can be played from the start, its moves follow the walk, and with `book deviate2` and `book deviate3` its total loss is the smallest. The mark `export=1` in the first answer is there.
- With the released `wEdax-x86-64-v3.exe`: the `book reduce` tests (500 cases), the 14 tests of damaged books and wrong words and the earlier tests of the analysis commands (3 small books, the real `book deviate` family, 40,000 random graphs of positions, the background thread, the lines of `book bestlines` and `book findline`) find nothing wrong; the unit tests of the analyses (`tests/book-analyze.cpp`) pass.
- The answers of 10 commands on the same positions (6,880 answers) are all the same as those of the C version v4.5.5-nikque.26 (except the time).
- Search: the 5 Windows release programs solve fforum-20-39 with 1 thread with the same results and node counts as the same check of v4.5.5-newedax.10: 78,733,223 with the `eval.dat` of eval2, 77,639,567 with that of Edax 4.5; the 7 Cassio cases pass. The 3 DLLs pass 220 API checks with 0 failures with both `eval.dat`. Android (NDK r27d): no warning.
- The search, the evaluation, the book learning and the file format are not changed (only the export and its mark in `book_analyze.cpp`, the reading of the command and `help book` in `edax.cpp`, and the function of libedax are new).
- Size (against v4.5.5-newedax.10): the 5 Windows programs +3,584 to +9,728, the 3 DLLs +20,480 to +22,016, the Android programs +7,464 and +3,656 and libraries +8,552 and +4,400 bytes (the functions of the export).
- The eval tools (`tools/eval`) are not changed, and their tests were not run again. The speed was not measured.

### Moved from the release notes

**Checked**: the same test of the export as the C version (4,545 cases on 3 test books).

## v4.5.5-newedax.10

Checks:

- The test of the order of the terms (the tests of the C version, `tests/book-analyze.cpp`): with `umigame-order`, the numbers and the `exact` marks are all the same when the terms are put in random orders before they are sorted. The earlier tests pass too, with the same numbers as the C version (39 in the test of dropped terms of `umigame-large` included); so do the tests of the precision stages (502,470 moves) and of the hint (24 cases).
- The tests of the analysis commands (the comparisons on 3 small books, the real `book deviate` family, 40,000 random graphs of positions, the background thread, the lines of `book bestlines` and `book findline`) all pass with the released `wEdax-x86-64-v3.exe`; so do the `book reduce` tests, 500 cases, and the 14 tests of damaged books and wrong words.
- The answers of 10 commands on the same positions (6,880 answers) are all the same as those of the C version v4.5.5-nikque.25 (except the time).
- Search: the 5 Windows release programs solve fforum-20-39 with 1 thread with the same results and node counts as v4.5.5-newedax.9: 78,733,223 with the `eval.dat` of eval2, 77,639,567 with that of Edax 4.5; the 7 Cassio cases pass. The 3 DLLs: 206 API checks, 0 failures, with both `eval.dat`. The 14 eval2 tools: built and tested without failure (compared with the C tools, only the seconds of a progress line of `label` differ; the files made are the same). No program, DLL or Android binary holds a folder of the build machine. The 4 Android binaries: no warning (not run on a device).
- The search, the evaluation, the book learning and the file format are not changed (only the order in which `book_analyze.cpp` drops the terms of an umigame number is changed).
- Size (against v4.5.5-newedax.9): `wEdax-x86-64-v4.exe` (no PGO) the same size, the 4 PGO programs −512 to +512 bytes, the 3 DLLs the same size; the Android programs +32 and +16 bytes, the libraries +32 and +16 bytes.
- The speed was not measured.

### Moved from the release notes

**Checked**: the tests of the C version (`tests/book-analyze.cpp`, `umigame-order` included) all pass, with the same numbers as the C version.

## v4.5.5-newedax.9

Checks:

- The tests of what is taken over (the tests of the C version, `tests/book-analyze.cpp`): the umigame numbers, the numbers of positions, `book levels` and `book missing` taken over are those found from each position alone, with the same numbers as the C version; the tests of the precision stages (502,470 moves) and of the hint (24 cases) pass. With a test build, the `@` lines of the commands of the GUI in 5 conditions on each position of 10 games of 2 test books are all the same as those of a program built from the source of v4.5.5-newedax.8 and as those of the C version.
- The tests of the analysis commands (the comparisons on 3 small books, the real `book deviate` family, 40,000 random graphs of positions, the background thread, the lines of `book bestlines` and `book findline`) all pass with the released `wEdax-x86-64-v3.exe`; so do the `book reduce` tests, 500 cases, and the 14 tests of damaged books and wrong words.
- The answers of 10 commands on the same positions (6,880 answers) are all the same as those of the C version v4.5.5-nikque.24 (except the time).
- Search: the 5 Windows release programs solve fforum-20-39 with 1 thread with the same results and node counts as v4.5.5-newedax.8: 78,733,223 with the `eval.dat` of eval2, 77,639,567 with that of Edax 4.5; the 7 Cassio cases pass. The 3 DLLs: 206 API checks, 0 failures, with both `eval.dat`. The 14 eval2 tools: built and tested without failure (compared with the C tools, only the seconds of a progress line of `label` differ; the files made are the same). No program, DLL or Android binary holds a folder of the build machine. The 4 Android binaries: no warning (not run on a device).
- The search, the evaluation, the book learning and the file format are not changed (only the code of `book_analyze.cpp` that finds the umigame numbers, the numbers of positions, `book levels` and `book missing` is changed).
- Size (against v4.5.5-newedax.8): `wEdax-x86-64-v4.exe` (no PGO) +1,536 bytes, the 4 PGO programs +512 to +2,560 bytes, the 3 DLLs the same size; the Android programs +1,488 and +848 bytes, the libraries +1,504 and +864 bytes.
- The speed was not measured.

### Moved from the release notes

**Checked**: with the tests of the C version (`tests/book-analyze.cpp`), the answers taken over are those found from each position alone, for every move. The `@` lines of the same commands as v4.5.5-newedax.8 (2 test books, 5 conditions) are the same as those of newedax.8 and of the C version.

## v4.5.5-newedax.8

Checks:

- The tests of the stages taken over and of the hint (the tests of the C version, `tests/book-analyze.cpp`): for 502,470 moves of 200 random books, the stages taken over are those walked from each position alone; the same on 2 test books (about 4.78 million moves on the book of 409,494 positions); the 24 cases of the hint pass. The answers of `book moveinfo` are the same as those of v4.5.5-newedax.7 and of the C version (2 test books, each position of 2 games, 3 bases).
- The tests of the analysis commands (the comparisons on 3 small books, the real `book deviate` family, 40,000 random graphs of positions, the background thread, the lines of `book bestlines` and `book findline`) all pass with the released `wEdax-x86-64-v3.exe`; so do the `book reduce` tests, 500 cases, and the 14 tests of damaged books and wrong words.
- The answers of 10 commands on the same positions (6,880 answers) are all the same as those of the C version v4.5.5-nikque.23 (except the time).
- Search: the 5 Windows release programs solve fforum-20-39 with 1 thread with the same results and node counts as v4.5.5-newedax.7: 78,733,223 with the `eval.dat` of eval2, 77,639,567 with that of Edax 4.5; the 7 Cassio cases pass. The 3 DLLs: 206 API checks, 0 failures, with both `eval.dat`. The 14 eval2 tools: built and tested without failure (compared with the C tools, only the seconds of a progress line of `label` differ; the files made are the same). No program, DLL or Android binary holds a folder of the build machine. The 4 Android binaries: no warning (not run on a device).
- The search, the evaluation, the book learning and the file format are not changed (the code changed finds the precision stages of `book moveinfo`, and writes the warning of an unknown command).
- Size (against v4.5.5-newedax.7): `wEdax-x86-64-v4.exe` (no PGO) +4,096 bytes, the 4 PGO programs +2,560 to +4,096 bytes, the 3 DLLs the same size; the Android programs +2,544 and +4,352 bytes, the libraries +2,392 and +4,368 bytes.
- The speed was not measured.

### Moved from the release notes

**Checked**: with the same tests as the C version (`tests/book-analyze.cpp`), the stages taken over are the stages walked from each position alone, for every move (200 random books and 2 test books). The answers of `book moveinfo` are the same as those of v4.5.5-newedax.7 and of the C version (2 test books, each position of 2 games, 3 bases).

## v4.5.5-newedax.7

Checks:

- The tests of `book reduce` and `book reducecount` (the tests of the C version) all agree, 546 cases (with 4 threads and 1 thread, with the count first, with the settings changed). The books saved after `book reduce` with the executables of v4.5.5-newedax.5 and of this version are the same byte for byte in 54 cases, with the count first too (but for the save date). The tests of damaged books and wrong words (14 cases) pass.
- The tests of the analysis commands (the comparisons on 3 small books, the real `book deviate` family, 40,000 random graphs of positions, the background thread, the lines of `book bestlines` and `book findline`) all pass with the released `wEdax-x86-64-v3.exe`; so do the `book reduce` tests, 500 cases, and the 14 tests of damaged books and wrong words.
- The answers of 10 commands on the same positions (6,880 answers) are all the same as those of the C version v4.5.5-nikque.22 (except the time).
- Search: the 5 Windows release programs solve fforum-20-39 with 1 thread with the same results and node counts as v4.5.5-newedax.6: 78,733,223 with the `eval.dat` of eval2, 77,639,567 with that of Edax 4.5; the 7 Cassio cases pass. The 3 DLLs: 206 API checks, 0 failures, with both `eval.dat`. The 14 eval2 tools: built and tested without failure, and the files made in the 26 comparisons with the C tools are all the same. No program, DLL or Android binary holds a folder of the build machine. The 4 Android binaries: no warning (not run on a device).
- The search, the evaluation, the book learning and the file format are not changed (the code changed runs only when `book reduce` or `book reducecount` is typed, or reads their settings).
- Size (against v4.5.5-newedax.6): `wEdax-x86-64-v4.exe` (no PGO) +6,144 bytes, the 4 PGO programs +3,584 to +4,608 bytes, the 3 DLLs +1,024 to +1,536 bytes. Android programs +3,512 and +6,408 bytes, libraries +3,624 and +6,552 bytes.
- With the real 29 GB book, the speed of `book reducecount` was measured with the C version only (the nikque.22 section).

## v4.5.5-newedax.6

Checks:

- The tests of `book reduce` and `book reducecount` (the tests of the C version) all agree, 546 cases, with 4 threads and with 1 thread. The books saved after `book reduce` with the executables of v4.5.5-newedax.5 and of this version are the same byte for byte in 54 cases (but for the save date). The tests of a damaged book and of wrong words (the 14 cases of the C version) all pass.
- The tests of the analysis commands (the comparisons on 3 small books, the real `book deviate` family, 40,000 random graphs of positions, the background thread, the lines of `book bestlines` and `book findline`) all pass with the released `wEdax-x86-64-v3.exe`; so do the `book reduce` tests, 500 cases, and the 14 tests of damaged books and wrong words.
- The answers of 10 commands on the same positions (6,880 answers) are all the same as those of the C version v4.5.5-nikque.21 (except the time).
- Search: the 5 Windows release programs solve fforum-20-39 with 1 thread with the same results and node counts as v4.5.5-newedax.5: 78,733,223 with the `eval.dat` of eval2, 77,639,567 with that of Edax 4.5; the 7 Cassio cases pass. The 3 DLLs: 206 API checks, 0 failures, with both `eval.dat`. The 14 eval2 tools: built and tested without failure (against the C tools, only the seconds of the progress line of `label` differ; the files made are the same; one test of the tools ran out of time once while the CPU was busy, and passed when run again). No program, DLL or Android binary holds a folder of the build machine. The 4 Android binaries: no warning (not run on a device).
- The search, the evaluation, the book learning and the file format are not changed (the code changed runs only when `book reduce` or `book reducecount` is typed).
- Size (against v4.5.5-newedax.5): `wEdax-x86-64-v4.exe` (no PGO) +8,704 bytes, the 4 PGO programs +4,608 to +5,120 bytes, the 3 DLLs the same size. Android programs +4,144 and +7,216 bytes, libraries +4,144 and +7,216 bytes.
- Not checked with the 29 GB real book.

## v4.5.5-newedax.5

Checks:

- The tests of `book reduce` and `book reducecount` (the tests of the C version: 3 small books made for the tests, compared with a computation written apart) all agree, 529 cases.
- The tests of the analysis commands (the comparisons on 3 small books, the real `book deviate` family, 40,000 random graphs of positions, the background thread, the lines of `book bestlines` and `book findline`) all pass with the released `wEdax-x86-64-v3.exe`; so do the `book reduce` tests, 500 cases.
- The answers of 10 commands on the same positions (6,880 answers) are all the same as those of the C version v4.5.5-nikque.20 (except the time).
- Search: the 5 Windows release programs solve fforum-20-39 with 1 thread with the same results and node counts as v4.5.5-newedax.4: 78,733,223 with the `eval.dat` of eval2, 77,639,567 with that of Edax 4.5; the 7 Cassio cases pass. The 3 DLLs: 206 API checks, 0 failures, with both `eval.dat`. The 14 eval2 tools: built and tested without failure (against the C tools, only the seconds of the progress line of `label` differ; the files made are the same). No program, DLL or Android binary holds a folder of the build machine. The 4 Android binaries: no warning (not run on a device).
- Speed: the search, the evaluation, the book learning and the file format are not changed (the code added runs only when a new command is typed). The speed of the new commands was not measured.
- Size (against v4.5.5-newedax.4): `wEdax-x86-64-v4.exe` (no PGO) +7,680 bytes, the 4 PGO programs +3,072 to +4,096 bytes, the 3 DLLs the same size. Android programs +3,120 and +7,056 bytes, libraries +3,248 and +7,104 bytes.
- Not checked with the 29 GB real book.

## v4.5.5-newedax.4

Checks:

- The tests of the analysis commands (the comparisons on 3 small books, 164 runs of the real `book deviate`, 40,000 random graphs of positions, the background thread) all pass.
- The answers of 10 commands on the same positions (6,880 answers) are all the same as those of the C version v4.5.5-nikque.19 (except the time).
- Search: 17 positions with 1 thread, level 14 and no book give the same depths, scores, node counts (7,676,900 in all) and principal variations as v4.5.5-newedax.2 (whose search v4.5.5-newedax.3 keeps). The 5 Windows release programs solve fforum-20-39 with 1 thread with the same results and node counts as v4.5.5-newedax.3: 78,733,223 with the `eval.dat` of eval2, 77,639,567 with that of Edax 4.5; the 7 Cassio cases pass. The 3 DLLs: 206 API checks, 0 failures, with both `eval.dat`. The 14 eval2 tools: built and tested without failure (against the C tools, only the seconds of the progress line of `label` differ; the files made are the same). No program, DLL or Android binary holds a folder of the build machine. The 4 Android binaries: no warning (not run on a device).
- Speed and memory of the new commands (a test book of 19.1 million positions, 1 thread, alternating with the C version): the number of positions from the start 5.03 s (C version 5.92 s), umigame numbers with loss 2/4 1.62 s (1.82 s), `book missing` deviate2 2 4 0.30 s (0.36 s). The analyses take 83 MB more memory (C version 86 MB); the book alone takes 880 MB (C version 1,235 MB). No command was slower than in the C version.
- Size (against v4.5.5-newedax.3): `wEdax-x86-64-v4.exe` (no PGO) +60,416 bytes, the 4 PGO programs +39,936 to +41,472 bytes, the 3 DLLs the same size. Android programs +31,568 and +52,288 bytes, libraries +32,032 and +53,152 bytes (the unused analysis code stays).
- Not checked with the 29 GB real book.

## v4.5.5-newedax.3

### Speed and memory (2026-10-19 to 20, measured)

One thread (one logical CPU, CPU cycles, 16 to 48 alternating pairs, ratio to the C version v4.5.5-eval2.6; in parentheses with the eval.dat of Edax 4.5, ratio to v4.5.5-nikque.18; the same node counts). Above 1, new edax is slower:

| Executable | fforum-20-39 | 30 middle-game positions (level 18) |
|---|---|---|
| `wEdax-x86-64-v4.exe` (no PGO) | 1.000 | **0.961 to 0.969** (0.979 to 0.981) |
| `wEdax-x86-64-v3.exe` (PGO) | 0.996 to 1.007 (0.998) | **1.008 to 1.011** (1.014 to 1.021) |
| `wEdax-x86-64.exe` (PGO) | 0.999 to 1.005 (1.002) | 1.000 to 1.005 (1.004) |
| `wEdax-x86-sse.exe` (PGO) | 0.990 to 0.991 (0.989) | 0.934 to 0.936 (0.953) |
| `wEdax-x86.exe` (PGO) | 0.921 to 0.926 (0.988) | 0.992 to 0.998 (0.985) |

32 threads (32 logical CPUs, book commands, ratio of the times to the C version v4.5.5-eval2.6, 6 rounds in the order A B B A):

| Command | `-v4` | `wEdax-x86-64.exe` | `-v3` |
|---|---|---|---|
| suite F (the `book deviate` family, level 21) | 0.978 | 0.987 | 0.999 |
| `book learn` (30 games, level 18) | 0.993 | 0.998 | **1.014** |
| `book fill 2` (level 12, a book of 270,000 positions) | 0.949 | 0.988 | **1.010** |
| `book fill2 2` | 0.960 | 0.990 | 1.006 |
| `book leaf expand` | 0.976 | 1.001 | 1.007 |
| `book fill2 2 5 5` | 0.978 | 0.999 | **1.013** |
| `book import` (a text book of 4 million positions) | 0.966 | 0.966 | 0.976 |

- **`wEdax-x86-64-v3.exe` stays about 1 % slower than the C version** (one-thread middle game, 32-thread `book learn` and `fill`). In its PGO build, 3 functions (the flips among them) stay called instead of inlined, and the evaluation function, with the same machine code, takes longer than in the C version. None of 9 tried changes helped (`docs/port/MEASURE-2.md`, chapter 2). On a CPU with AVX-512, use `wEdax-x86-64-v4.exe` (faster than the C version).
- Peak memory: the same as the C version or less (`book import` 614 -> 545 MiB).
- Loading a book of 34.7 million positions (made by the C version) from an HDD: 0.914 times the time of the C version, CPU time 8.3 -> 3.8 s, peak memory 3,243 -> 1,667 MiB.
- The eval2 tools: `w2dat2` and `label` about 0.7 % slower than the C tools (cause not found); the others the same or faster.

### Checks

- The 5 Windows executables: one-thread `-solve` of fforum-20-39 gives the same depths, scores, node counts and lines as v4.5.5-eval2.5 with `bin/data/eval.dat` (eval2; 78,733,223), and as v4.5.5-nikque.17 with the `eval.dat` of Edax 4.5 (77,639,567). The 7 Cassio cases pass.
- The 3 DLLs: 206 API checks, 0 failures each, with both `eval.dat`. The same machine code as v4.5.5-newedax.2.
- The books of 14 test suites (the machine code of `wEdax-x86-64-v4.exe`, one thread and 32 threads): the same as the records; only the one-thread run of S3 with the default settings differs, as its book seed is the clock (as before).
- The 14 eval2 tools: all tests pass; on 26 cases their outputs are the same as those of the C tools.
- Android (NDK r27d, armeabi-v7a and arm64-v8a): built without warnings, the same size as v4.5.5-newedax.2; not run on a device.
- None of the 26 executables, libraries, tools and Android binaries holds a folder of the build machine (`tests/check-build-paths.py`).
- Size: `-v4` 824,832, `-v3` 698,368, `wEdax-x86-64.exe` 715,264, `wEdax-x86-sse.exe` 642,560, `wEdax-x86.exe` 692,224 bytes (the 4 PGO builds 48 to 144 KB smaller than in v4.5.5-newedax.2); the DLLs the same. Against the C version v4.5.5-eval2.6: the 4 PGO builds +18 to 30 KB, `-v4` +163 KB (no PGO).
- The released `-v3`, `wEdax-x86-sse.exe` and `wEdax-x86.exe` differ a little from the measured builds (a PGO build changes from a build to the next), so the released ones were measured again against the C version (the table above). `-v4`, `wEdax-x86-64.exe` and the DLLs have the machine code of the measured ones.

## v4.5.5-newedax.2

### Speed and memory (2026-10-19, measured)

Windows, the released `wEdax-x86-64-v4.exe` against each other, 32 threads (32 logical CPUs), the `config.ini` of the release. Ratio of the times (the newer one / the one compared; below 1 is faster): the mean and standard deviation of 6 rounds in the order A B B A.

| Command | against v4.5.5-newedax.1 | against the C version v4.5.5-eval2.6 |
|---|---|---|
| suite F (`book deviate`, `deviate2`, `deviate3`, `enhance`, level 21) | 1.006 ± 0.014 | **0.985 ± 0.012** |
| `book learn` (30 games, level 18) | 0.997 ± 0.009 | 1.001 ± 0.022 |
| `book fill 2` (level 12, a book of 270,000 positions) | 0.988 ± 0.009 | **0.959 ± 0.008** |
| `book fill2 2` | 0.999 ± 0.012 | **0.973 ± 0.011** |
| `book leaf expand` | 0.991 ± 0.012 | **0.967 ± 0.010** |
| `book fill2 2 5 5` | 0.987 ± 0.013 | 0.979 ± 0.024 |
| `book import` (a text book of 4 million positions) | 0.996 ± 0.020 | **0.963 ± 0.010** |
| fforum-20-39 (`-solve`) | — | 0.995 ± 0.028 |
| 30 middle-game positions (level 18, `-solve`) | — | 0.986 ± 0.011 |

- With 32 threads and `wEdax-x86-64-v4.exe`, **no case slower than v4.5.5-newedax.1 or than the C version was found** (every difference is within the standard deviation or faster; the 1.006 of suite F against newedax.1 has a median of 1.002, 3 of the 6 rounds slower). The C version is a PGO build, new edax is not.
- Peak memory: the same as v4.5.5-newedax.1 (±1 MiB); the same as the C version v4.5.5-eval2.6 or less (`book import`: 614 MiB → 544 MiB).
- **With one thread, the 4 executables other than `wEdax-x86-64-v4.exe` are slower than the C version** (one logical CPU, CPU cycles, 16 alternating pairs, ratio to the C version v4.5.5-eval2.6, the same node counts):

  | Executable | fforum-20-39 | 30 middle-game positions (level 18) |
  |---|---|---|
  | `wEdax-x86-64-v4.exe` | 1.000 | 0.966 |
  | `wEdax-x86-64-v3.exe` | **1.008** | **1.017** |
  | `wEdax-x86-64.exe` | **1.008** | **1.025** |
  | `wEdax-x86-sse.exe` | **1.010** | **1.116** |
  | `wEdax-x86.exe` | 0.936 | **1.047** |

  The releases of the C version are PGO builds, new edax is not. Without PGO on both sides (the same options), new edax is as fast or faster (middle game: `-v3` 0.992, `wEdax-x86-64.exe` 0.998). A PGO build of new edax narrows the gap, but the middle game is still slower than the C version (`-v3` 1.008, `wEdax-x86-64.exe` 1.013, `wEdax-x86-sse.exe` 1.026): PGO inlines other functions into their callers than in the C version (`get_moves` and `search_eval_1` are inlined into `NWS_shallow` in the C version, seen by sampling); this is presumably the cause (not checked). The next version fixes it.
- `wEdax-x86.exe` (32-bit, no SSE) is as fast as in v4.5.5-newedax.1 (0.9995, one logical CPU, 40 pairs).
- Size: the Windows executables are 74 to 173 KB larger than those of the C version v4.5.5-eval2.6, the DLLs about 27 KB. Most of it comes from the build without PGO (without PGO on both sides: about 32 KB larger: the new features such as the files of form 2, the sort and the tables of Boost).

### Checks

- The 5 Windows executables: one-thread `-solve` of fforum-20-39 gives the same depths, scores, node counts and lines as v4.5.5-eval2.5 with `bin/data/eval.dat` (eval2; 78,733,223 nodes), and as v4.5.5-nikque.17 with the `eval.dat` of Edax 4.5 (77,639,567). The 7 Cassio cases pass.
- The 3 DLLs: 206 API checks, 0 failures each, with both `eval.dat`.
- The 14 eval2 tools: 26 tests pass; on 26 cases their outputs are the same as those of the C tools.
- No executable, library or tool holds a folder of the build machine.
- Android (NDK r27d, armeabi-v7a and arm64-v8a): built without warnings; not run on a device.
- Size: the same as v4.5.5-newedax.1 (`libedax-x64.dll` 512 bytes smaller). Android: 288 to 448 bytes larger (the code of the fixes).

## v4.5.5-newedax.1

Checks:

- With the same options as the C version v4.5.5-nikque.17 (no PGO): no case slower than the C version in fforum-20-39 (1 and 32 threads), fforum-40-59, 30 middle-game positions (levels 18 and 21), suite F, or the start (`docs/port/MEASURE-1.md`).
- The same books as the C version in 14 test suites (with the same random seed), the same C API of libedax and Cassio protocol; 53 regression cases.

Not checked yet (also for newedax.2): builds other than MSVC on Windows and the Android NDK, an Android device, AddressSanitizer and ThreadSanitizer, real books of billions of positions, a review by a second reader.
