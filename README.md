# Heath (formerly newedax)

Heath (formerly newedax) is the Othello engine [Edax 4.5.5 nikque](https://github.com/Nikque/edax-reversi-AVX) (its eval2 line) ported to C++23.

- Since v4.5.5-newedax.13 it has a new evaluation function, trained again with 64 feature slots (and a 48-slot build).
- With the same time per game, it is about 60 Elo stronger than with the eval2 evaluation.
- It uses less memory for books than the C version.

[日本語](#日本語) · [Download](https://github.com/Nikque/Heath/releases/latest) · [Manual](manual/en/README.md) · [Release notes](RELEASE-NOTES.md) · [Tests and measurements](docs/validation/newedax.en.md)

## Download and run

1. Download a ZIP from the [latest release](https://github.com/Nikque/Heath/releases/latest) and unpack it. There are two ZIPs:
   - `edax-…-64.zip`: the default, 64 slots.
   - `edax-…-48.zip`: 48 slots, faster per node but weaker with the same time per game.

   Each contains executables for Windows (x64, x86) and Android, the libedax DLLs, the training tools, and the `eval.dat` of its build.
2. Open the `bin` folder and start the executable from **inside that folder**. On Windows, start with `wEdax-x86-64.exe`; use `-v3` or `-v4` if your CPU supports it. `-v3` needs a CPU of the x86-64-v3 level (AVX2, BMI1, BMI2, FMA, LZCNT, MOVBE: Intel Haswell or AMD Zen and later), `-v4` one of the x86-64-v4 level (AVX-512 F, BW, CD, DQ and VL as well: Intel Skylake-SP or Ice Lake, AMD Zen 4 and later).

The `bin/` folder of the repository holds only the data: `book.dat`, `config.ini` and the `eval.dat` of eval2. That `eval.dat` is not for the builds since v4.5.5-newedax.13: they read it with a warning, but their evaluation is then wrong. The `eval.dat` of the 64 and of the 48 slots are in the release ZIPs: use the one of the ZIP of your build.

Most commands and settings are the same as the C version: see the [manual](manual/en/README.md). The settings that differ are in [Differences from the C version](#differences-from-the-c-version).

For Linux and macOS executables, use the C version ([edax-reversi-AVX releases](https://github.com/Nikque/edax-reversi-AVX/releases)).

## Relation to the C version

- Evaluation: since v4.5.5-newedax.13, a new evaluation of 64 slots (or 48), with its own `eval.dat`; the probcut model is `refit` (fixed since v4.5.5-heath.20). Up to v4.5.5-newedax.12 it was eval2, with the same results as the C version.
- Most commands and settings are the same as the C version (the differences are in the next section); the search results differ (the evaluation and the probcut).
- **v4.5.5-heath.19** includes C v4.5.5-nikque.32 and v4.5.5-eval2.19. Exact endgame solves are faster (98.2% of the time on fforum-40-59 with one thread), with the same results.
- **v4.5.5-heath.18** includes C v4.5.5-nikque.31 and v4.5.5-eval2.19. It fixes the game count summary, estimate errors, protocol handling, and command help.
- **v4.5.5-heath.17** includes C v4.5.5-nikque.30 and v4.5.5-eval2.18. It fixes an exact-endgame bound that could be wrong when a transposition key ignored the color of solid discs.
- Each Heath release contains all changes of a C release. **v4.5.5-heath.16** contains C v4.5.5-nikque.29 / v4.5.5-eval2.17; `book correct`, `book leaf recalculate` and `book learn` go on from a book saved before they ended, and `book deepen` works (v4.5.5-newedax.15 fixed a rare wrong score of the exact search from 17 empties on; v4.5.5-newedax.14 added thinking on the opponent's time under NBoard, `-ponder on`, and the hash tables kept from move to move, `-keep-hash on`).

## Differences from the C version

- **Memory**: a book position takes 40 bytes (48 in the C version). For 6.49 million positions, loading and saving with one thread peaked at 364 versus 665 MiB, or 362 versus 408 MiB in the other record order. These reductions range from 11% to 45%. The comparison used nikque.17 and `f8b4d1f`, 45 commits before the newedax.1 tag. [Measurement record](docs/port/MEASURE-1.md)
- **Settings**: only Heath has `book-format` (the form of the saved books: `1`, `2` or `auto`) and `keep-hash`. Only the C version has `probcut-model`, `pc-slope`, `pc-dev` and `pc-mob`: Heath has the probcut model `refit` and the values of `pc-slope 1.3,12`, `pc-dev 2,0.04,0.75` and `pc-mob 2.219,-0.184,-0.350` built in (since v4.5.5-heath.20). The default of `lazy` is `auto` in Heath, `0` in the C version.
- **Evaluation files**: each build needs the `eval.dat` of Heath of its slot count (in the release ZIP of that build). The `eval.dat` of eval2 (`bin/data/eval.dat` of the repository) and that of Edax 4.5 are read with a warning, and the evaluation is then wrong.
- **Strength** (v4.5.5-newedax.13, 64 slots, games without a book from random openings, each played with both colours): +60.2 Elo against eval2 (v4.5.5-newedax.12; one thread each, 30 s a game a side, 1,102 games); +23.2 Elo against Egaroucid (tournament branch, 120 s a game, 8 threads, 180 games). See the [release notes](RELEASE-NOTES.md).
- **Speed** (eval2, up to v4.5.5-newedax.12; Windows, against the C release eval2.6): `wEdax-x86-64-v4.exe` is slightly faster; the x64 standard and x86 builds are as fast or faster; **`wEdax-x86-64-v3.exe` is about 1 % slower**.
- **New file format (form 2)** for books (`NEDXBOOK`, about 33 % smaller) and evaluation files (`NEDXEVAL`), with a checksum. Convert with `-book-convert` / `-eval-convert` (both ways). Form 1 files (C version) are still read.
- **Build options**: `EVAL_SLOTS` 64 (default) or 48; `EVAL_MOBILITY` (default 1).
- **[tools/eval](tools/eval)**: the training tools in C++ (`evsearch`, `evtrain`, `w2dat2`, `selfplay3`, ...).

## Not checked yet

Builds other than MSVC on Windows (Android: compiled and linked, not run on a device); AddressSanitizer / ThreadSanitizer; real books of billions of positions; review by a second person.

## Build

- Boost 1.92 (headers only) in `../third_party/boost_1_92_0`, or set `BOOST_DIR=<directory>`.
- Windows (Visual Studio 2022, x64 Developer Command Prompt): `build-win-v4.cmd`, or in `src`: `nmake -f NMakefile vc-x64-v3` (other targets and PGO builds: `src/NMakefile`).
- Release binaries: `tools/port/build-release.cmd <checkout> <log directory>`. It builds `wEdax-x86-64-v4.exe` and the 3 DLLs without PGO. In the v4.5.5-heath.21 ZIPs, replaced on 2026-11-26, these are PGO builds too: `-v4` with the same profile as the other 4 executables, the DLLs trained with `bench 20` and the API tests of `tests/libedax_test.c`. `src/NMakefile` has no PGO target for the DLLs.
- Release ZIP: `python package-release.py <zip> --slots 64|48 --eval <eval.dat>`. `--slots` is the slot count of the binaries; `--eval` is the `eval.dat` put in the ZIP. The script checks that this `eval.dat` holds the weights of that slot count.
- Android: NDK r27d, `ndk-build`.

Port records (in Japanese): [docs/port](docs/port).

## License

GPL-3.0 ([LICENSE](LICENSE)). Based on Edax by Richard Delorme and [okuhara/edax-reversi-AVX](https://github.com/okuhara/edax-reversi-AVX). Boost is under the Boost Software License and is not included.

---

## 日本語

Heath（旧名 newedax）は、オセロのエンジン [Edax 4.5.5 nikque](https://github.com/Nikque/edax-reversi-AVX)（eval2 の系列）を C++23 に移したものです。

- v4.5.5-newedax.13 から、評価関数を 64 枠（と 48 枠の版）で学習し直した新しいものにしました。
- 同じ持ち時間で、eval2 の評価関数より約 60 Elo 強くなりました。
- book のメモリは C 版より少なくて済みます。

[ダウンロード](https://github.com/Nikque/Heath/releases/latest) · [マニュアル](manual/ja/README.md) · [修正一覧](RELEASE-NOTES.ja.md) · [テストと実測の記録](docs/validation/newedax.ja.md)

### 使い始め方

1. [最新の Release](https://github.com/Nikque/Heath/releases/latest) の ZIP をダウンロードして展開します。ZIP は 2 種類あります。
   - `edax-…-64.zip`：既定の 64 枠。
   - `edax-…-48.zip`：48 枠。1 節点あたりは速いが、同じ持ち時間では弱い。

   どちらにも、Windows（x64・x86）と Android の実行ファイル、libedax の DLL、学習ツール、その版の `eval.dat` が入っています。
2. `bin` フォルダーを開き、**そのフォルダーの中から**実行ファイルを起動します。Windows なら、まず `wEdax-x86-64.exe` を使ってください。CPU が対応していれば `-v3`・`-v4` が速くなります。`-v3` は x86-64-v3 の水準の CPU（AVX2・BMI1・BMI2・FMA・LZCNT・MOVBE。Intel は Haswell、AMD は Zen 以降）、`-v4` は x86-64-v4 の水準の CPU（さらに AVX-512 の F・BW・CD・DQ・VL。Intel は Skylake-SP・Ice Lake、AMD は Zen 4 以降）が要ります。

リポジトリの `bin/` にはデータ（`book.dat`、`config.ini`、eval2 の `eval.dat`）だけがあります。eval2 の `eval.dat` は v4.5.5-newedax.13 からの版には使えません（警告を出して読みますが、評価が正しくなりません）。64 枠と 48 枠の `eval.dat` は Release の ZIP に入っています。使うビルドの ZIP のものを使ってください。コマンドと設定のほとんどは C 版と同じです（[マニュアル](manual/ja/README.md)）。違う設定は [C 版との違い](#c-版との違い) にあります。

Linux・macOS 用の実行ファイルは C 版（[edax-reversi-AVX の Releases](https://github.com/Nikque/edax-reversi-AVX/releases)）を使ってください。

### C 版との関係

- 評価関数：v4.5.5-newedax.13 から、64 枠（または 48 枠）の新しい評価関数で、専用の `eval.dat` を使います。ProbCut は `refit` です（v4.5.5-heath.20 から固定）。v4.5.5-newedax.12 までは eval2 で、C 版と同じ結果でした。
- コマンドと設定のほとんどは C 版と同じです（違いは次の節）。探索の結果は評価関数と probcut が違うので変わります。
- **v4.5.5-heath.19** には、C 版 v4.5.5-nikque.32 と v4.5.5-eval2.19 の変更が入っています。終盤の完全読みが速くなりました（1 スレッドの fforum-40-59 で 0.982 倍の時間）。結果は前の版と同じです。
- **v4.5.5-heath.18** には、C 版 v4.5.5-nikque.31 と v4.5.5-eval2.19 の変更が入っています。棋譜の完了件数、見積もりの誤差、外部プロトコルとヘルプを修正しました。
- **v4.5.5-heath.17** には、C 版 v4.5.5-nikque.30 と v4.5.5-eval2.18 の変更が入っています。度外石の色を置換表のキーに反映せず、終盤の完全読みで探索境界がずれることがあった不具合を修正しました。
- Heath の各版は、C 版のある版の変更をすべて含みます。**v4.5.5-heath.16** は、C 版 v4.5.5-nikque.29・v4.5.5-eval2.17 を含みます。`book correct`・`book leaf recalculate`・`book learn` が途中で保存された book から続けられるようになり、`book deepen` が働くようになりました。v4.5.5-newedax.15 では、空き 17 マス以上の完全読みで、まれに評価値を誤っていたのを直しました（v4.5.5-newedax.14 で、NBoard で相手の時間にも考える `-ponder on` と、置換表を手から手へ残す `-keep-hash on` を足しました）。

### C 版との違い

- **メモリ**：book の1局面は40バイトです（C版は48バイト）。649万局面を1スレッドで読み込んで保存したとき、最大使用量は665→364 MiB、別の格納順では408→362 MiBでした。削減率は11〜45%です。比較対象は nikque.17 と、newedax.1 のタグより45コミット前の `f8b4d1f` です。[計測記録](docs/port/MEASURE-1.md)
- **設定**：`book-format`（保存する book の形式。`1`・`2`・`auto`）と `keep-hash` は Heath だけにあります。`probcut-model`・`pc-slope`・`pc-dev`・`pc-mob` は C 版だけにあります。Heath は v4.5.5-heath.20 から、probcut の型 `refit` と、`pc-slope 1.3,12`・`pc-dev 2,0.04,0.75`・`pc-mob 2.219,-0.184,-0.350` の値を作り付けで使います。`lazy` の既定は Heath が `auto`、C 版が `0` です。
- **評価ファイル**：ビルドには、その枠の数の Heath の `eval.dat`（そのビルドの Release の ZIP のもの）が要ります。eval2 の `eval.dat`（リポジトリの `bin/data/eval.dat`）と Edax 4.5 のものは、警告を出して読みますが、評価が正しくなりません。
- **強さ**（v4.5.5-newedax.13、64 枠。book なし、ランダムな開始局面から先手・後手を入れ替えて対局）：eval2（v4.5.5-newedax.12）に +60.2 Elo（双方 1 スレッド、1 局 30 秒ずつ、1,102 局）、Egaroucid（大会用の枝、1 局 120 秒、8 スレッド、180 局）に +23.2 Elo。[修正一覧](RELEASE-NOTES.ja.md)を見てください。
- **速さ**（eval2、v4.5.5-newedax.12 まで。Windows、C 版 eval2.6 の配布物と比較）：`wEdax-x86-64-v4.exe` はわずかに速く、x64 標準版・x86 版は同じか速い。**`wEdax-x86-64-v3.exe` は約 1% 遅い**です。
- **新しいファイル形式（形式 2）**：book（`NEDXBOOK`、約 33% 小さい）と評価ファイル（`NEDXEVAL`）。チェックサム付き。`-book-convert`・`-eval-convert` で相互に変換できます。C 版の形式 1 のファイルもそのまま読めます。
- **ビルドオプション**：`EVAL_SLOTS` 64（既定）か 48、`EVAL_MOBILITY`（既定 1）。
- **[tools/eval](tools/eval)**：学習ツールの C++ 版（`evsearch`・`evtrain`・`w2dat2`・`selfplay3` など）。

### まだ確認していないこと

Windows の MSVC 以外でのビルド（Android はリンクまで、実機では未実行）、AddressSanitizer・ThreadSanitizer での試験、数十億局面の実際の book、別の人によるレビュー。

### ビルド

上の英語の「Build」を見てください。配布の ZIP は `python package-release.py <zip> --slots 64|48 --eval <eval.dat>` で作ります。`--slots` は実行ファイルの枠の数、`--eval` は ZIP に入れる `eval.dat` で、スクリプトはこの `eval.dat` がその枠の数の重みかを確かめます。移植の作業記録は [docs/port](docs/port) にあります。

配布の実行ファイルを作る `tools/port/build-release.cmd` は、`wEdax-x86-64-v4.exe` と DLL 3 本を PGO なしでビルドします。2026-11-26 に差し替えた v4.5.5-heath.21 の ZIP では、これらも PGO 版です。`-v4` はほかの 4 本と同じプロファイル、DLL は `tests/libedax_test.c` の `bench 20` と API のテストで学習しました。DLL の PGO の対象は `src/NMakefile` にありません。

### ライセンス

GPL-3.0（[LICENSE](LICENSE)）。Richard Delorme 氏の Edax と [okuhara/edax-reversi-AVX](https://github.com/okuhara/edax-reversi-AVX) をもとにしています。Boost は Boost Software License で、このリポジトリには含みません。
