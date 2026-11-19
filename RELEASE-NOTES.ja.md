# Heath（旧名 newedax）修正一覧

[English](RELEASE-NOTES.md) · [README](README.md) · [マニュアル](manual/ja/README.md) · [テストと実測の記録](docs/validation/newedax.ja.md)

Heath（旧名 newedax。C 版 Edax 4.5.5 nikque の eval2 系列を C++23 に移したもの）の、版ごとの使う人に関わる変更点です。テストと実測の詳しい記録は、各版の末尾のリンク先にあります。

C 版から取り込んだ変更の詳細は、[C 版の修正一覧](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.ja.md) を見てください。

## C 版との対応

各版は、同じ行の C 版の変更をすべて含みます。

| Heath（newedax） | C 版（nikque） | C 版（eval2） |
|---|---|---|
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

## v4.5.5-heath.17

[v4.5.5-nikque.30](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.ja.md#v455-nikque30) の変更を取り込みました。SVE の評価更新は 64 枠版と 48 枠版の両方に対応します。

**不具合の修正**

- 度外石の色をそろえた盤面を終盤の置換表のキーに使うと、勝者が変わる局面で空きマスの配点を誤り、完全読みの探索境界がずれることがありました。実際の盤面をキーに使うようにしました。

**互換性**

- 修正の影響を受ける局面では、探索結果や探索ノード数が変わることがあります。評価関数、`eval.dat`、book のファイル形式は前の版と同じです。

**未確認**。対局での強さ、実 book、SVE の実機での動作と速度。

[検証記録](docs/validation/newedax.ja.md#v455-heath17)

---

## v4.5.5-heath.16

Heath（旧名 newedax）の名前での最初の版です。`book correct`・`book leaf recalculate`・`book learn` が途中で保存された book から続けられるようになり、`book deepen` が働くようになりました。

**変更**

- book の 4 つのコマンドの変更は、C 版 v4.5.5-nikque.29・v4.5.5-eval2.17 の変更です（[C 版の修正一覧](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.ja.md#v455-nikque29)）。
- `book correct`：確かめ直した完全読みの局面に、book の中でマークを付けます。
- 途中で保存された book（`<book-file>.err`、終了のときの保存）で `book correct` をもう一度実行すると、マークのない局面だけを探索します。全部終わると、マークをすべて外します。
- v4.5.5-newedax.15 までの `book correct` は、もう一度実行すると全部を最初から探索し直していました（大きな book では何日もかかります）。
- `book leaf recalculate`（2・3・4）：同じく、計算し直した Leaf にマークを付け、続きから計算します。
- `book learn <ファイル>`：`<book-file>.learn` を書きます。中身は、`<book-file>.store` に保存した book に学習済みの行の数、その確かめの値、book の局面の数・level・深さです。
- 同じファイルでこの book に `book learn` をもう一度実行すると、学習済みの行を飛ばします。最後まで学習すると `<book-file>.learn` は消えます。
- `book deepen`：level が book より浅い局面に book の level を与え、その level で探索し直します。これまでは局面の level のまま探索し直していたため、何も深くならず、もう一度実行すると全部をやり直していました。
- 名前：README・修正一覧・マニュアルで Heath（旧名 newedax）にしました。リポジトリは `Nikque/Heath` になりました（前のアドレスからも移ります）。
- プログラムは、これまでどおり自分を Edax と名乗ります（GUI が使う通信の中）。前の版の名前（`v4.5.5-newedax.N`）は変わりません。

**互換性**

- マークは、局面の level の 1 バイトの上 2 ビットです（book のファイルの 2 つの形式とも）。ファイルの大きさと形式は変わりません。マークのない book は前と同じです。
- **`book correct`・`book leaf recalculate` が終わる前に保存された book にはマークがあり、v4.5.5-newedax.15 までの版では壊れた book とみなされ、読み込めません。** 終わった book にはマークがありません。
- 評価のツール（`tools/eval`）は、マークを外して level を読みます。
- 探索は変わりません。v4.5.5-newedax.15 と同じ評価値・ノード数・読み筋です。
- 評価関数とその `eval.dat` は変わりません（v4.5.5-newedax.13 のもの。この版の ZIP に入っています）。

**未確認**：実際の大きな book、Android の実機、Windows ARM64。

[検証記録](docs/validation/newedax.ja.md#v455-heath16)

---

## v4.5.5-newedax.15

完全読みの不具合を直しました。空き 17 マス以上の level 60 で、まれに評価値（と手）を誤っていました。

**不具合の修正**

- 完全読み（深さが空きマス数で、選択なしの level）で、根の評価値と手を誤ることがありました（誤りはいつも数石）。
- 新しい `eval.dat` の学習データでは、空き 17 マスの完全読みの 10 万回に 1〜2 回ほど誤っていました。1 スレッドでも 4 スレッドでも、置換表の大きさによらず、同じ局面で毎回誤ります。
- 原因：完全読みは、まず選択あり（probcut）で読み、その後に完全に読みます。その間、中盤の探索で終盤の探索に移る深さ（`DEPTH_MIDGAME_TO_ENDGAME`、空き 15）の節を、正確な値として置換表に残していました（子が終盤の探索で読まれるため）。
- その節がパスのときは、相手の探索が中盤の探索の同じ深さのまま続き、probcut で打ち切られることがあります。その値が正確な値として残り、完全な読みの反復で使われていました。
- パスの節は、読みの選択の強さのまま残すようにしました。
- 同じ誤りは C 版（v4.5.5-nikque.27・v4.5.5-eval2.15）にもあり、v4.5.5-nikque.28・v4.5.5-eval2.16 で直しました（[C 版の修正一覧](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.ja.md#v455-nikque28)）。
- その元の奥原俊彦氏の Edax 4.5.5・4.5.6（edax-reversi-AVX）にもあります。Richard Delorme 氏の Edax 4.6・4.7 にはありません。

**互換性**

- 正しかった完全読みの評価値は変わりません。
- ノード数は少し変わります（空き 17〜22 マスの 1,640 局面で 0.01% 増）。
- 終盤の深さを含まない中盤の探索は変わりません。
- 前の版で完全読みした局面を含む book には、まれにこの誤りの値が入っていることがあります。
- ほかの探索・評価関数とその `eval.dat`（v4.5.5-newedax.13 のもの。この版の ZIP に入っています）・book は変わりません。

**未確認**：Android の実機、Windows ARM64。

[検証記録](docs/validation/newedax.ja.md#v455-newedax15)

---

## v4.5.5-newedax.14

NBoard で相手の時間にも考えられるようにし、置換表を手から手へ残せるようにしました。どちらも新しい設定で、指定しなければ切っています。

**変更**

- `-ponder on`（または `set ponder on`）：NBoard のプロトコルで、相手の時間にも考えます。
  - Edax の `go` の手を GUI が `move` で返すと、Edax は相手の時間に考えます。相手の手を（直前の探索から）予想し、その後の局面を探索します。
  - 相手の手が `move`、またはその後の局面の `set game` で来て、予想と同じなら、その探索を続けます（次の `go` が続きを使います）。違う手なら止め、`go` はこれまでどおり探索します。
  - `ping` は相手の番の思考を止めません。`go` の探索は、これまでどおり止めます。
  - `-ponder on` を指定しないと、NBoard ではこれまでどおり考えません。`ponder` はほかのプロトコルのため既定が on なので、NBoard では指定したときだけ効きます。
- `-keep-hash on`（または `keep-hash on`、NBoard では `set keep-hash on`。既定は off）：新しい盤が同じ対局の続きなら、置換表を残します。
  - 同じ対局の続きとは、前の盤で石のあるマスに、新しい盤でもすべて石があることです。
  - NBoard の `set game`、Edax のプロトコルの `setboard` と FEN、libedax の盤に効きます。
  - v4.5.5-newedax.13 までは、新しい盤のたびに置換表を消していました（記録がすべて無効になっていました）。そのため、手ごとに盤を送る GUI では毎手が空の表からの探索になり、人の番の探索（nikque GUI の「相手の番にも考える」）も次の手に生きていませんでした。
  - NBoard では、相手の番に考えた局面が来たときは、この設定によらず置換表を残します。
- `-ponder on -keep-hash on` で NBoard を使う GUI のコマンドの並び：

```
nboard 1
set depth 60
ping 1                 → pong 1
set game <GGF>         局面（Edax の手番）
go                     → === <手> <評価値> <時間>   （時間が来たら "ping 2"：=== がすぐ返り、その後に pong 2）
move <手>              === の手：Edax は相手の時間に考える
ping 3                 → pong 3   （相手の番の思考は止まらない）
set game <GGF>         相手の手の後の局面（または move <相手の手>）
go                     → 相手が予想した手を打ったときは、相手の番に始めた探索を続ける
```

- 新しいテスト `tests/nboard-ponder.py`：この版の変更を `-info` の標準エラーで確かめます。
- マニュアル：設定 `ponder` と `keep-hash`（4.6、14）、NBoard での相手の番の思考（12.1）。

**不具合の修正**

- NBoard の `go` の直後、探索が始まる前に来た `ping` でも、探索が止まるようにしました。v4.5.5-newedax.13 までは、この止めが失われ、探索が自分の限りまで続きました。

**互換性**

- `-ponder on` と `-keep-hash on` を指定しなければ、答えは v4.5.5-newedax.13 と同じです。違うのは、`go` の直後の `ping` で探索が止まることだけです。
- `-keep-hash on` では、前の手の探索で見つけたものを使うので、評価値やノード数は空の表からの探索と同じになりません。
- NBoard で `-ponder on` にすると、相手の時間にもスレッドを使います。同じ CPU でほかのエンジンを動かすときは、そのエンジンの時間を取ります。
- 探索・評価関数・`eval.dat`（v4.5.5-newedax.13 のもの。この版の ZIP に入っています）・book は変わりません。

**未確認**：相手の番の思考のありとなしの強さ（この版では対局していません）、NBoard の GUI そのもの、Android の実機、Windows ARM64。

[検証記録](docs/validation/newedax.ja.md#v455-newedax14)

---

## v4.5.5-newedax.13

新しい評価関数にしました。64 枠で学習し直したもの（既定）と、48 枠の版の 2 つです。64 枠の版は、同じ持ち時間で eval2（v4.5.5-newedax.12）に +60.2 Elo でした。

**変更**

- 配布版は 2 つの ZIP です。
  - `edax-4.5.5-newedax.13-64.zip`：既定の 64 枠。
  - `edax-4.5.5-newedax.13-48.zip`：48 枠。64 枠から形を 2 つ外したもので、1 節点あたりが速くなります。
  - それぞれに専用の `eval.dat`（107,612,578 バイト）が入っています。
- 評価関数（既定の版、`EVAL_SLOTS` 64）：Edax 4.5 の 13 群と追加の 13 群（`M X W C`、3^10 マスまたは 3^9 の重み）、15 個の形、着手可能数の重みを 64 枠に置きました。
- 形は 2026-11-05〜11-13 に、評価の誤差と Lv18・Lv24 の対局で探しました（形とマスは次の表）。
- 重みは `tools/eval/evsearch`（手数 2〜53）で学習し直しました。
- 48 枠の版（`EVAL_SLOTS=48`）：64 枠と同じソース・同じ形式の `eval.dat` で、形 `cxd10` と `edw10`（16 枠）を外したものです。重みはこの 2 つを外して学習しました。
- 48 枠の版の特徴の差分更新は、256 ビットのベクトル 4 本のかわりに 3 本です。同じ時間で約 11% 多く読みますが、評価の落ちのほうが大きく、同じ時間で 64 枠に −12.1 Elo（−17.3〜−6.9）でした。
- `EVAL_SLOTS` 48 は eval2 の意味ではなくなりました。eval2 の評価関数（46 パターン）はビルドできません。
- ProbCut：既定を `refit` にしました（`-probcut-model refit`、`bin/config.ini` の `probcut-model = refit`）。
- 探索：待っているスレッドが眠る前に見回るようにし、深さ 9 から分けます。
- AVX-512 の版は、手数ごとの重みを 1 つの表から 1 つずつ読みます。
- ツール（`tools/eval`）：`evshape`、`evsearch -mute`・`-method sd`、`w2dat2 -interp` と追加の群、`selfplay3 -h`、`selfplay3` と `label` の探索の refit、`pcsigma`。
- 評価ファイル：64 枠・48 枠の版は、追加の群の重みの無いファイル（eval2 や Edax 4.5 の `eval.dat`）を読むと警告を出します。
- テスト（`tests/regression.cpp`）：`evalslots` が AVX-512 の版の表も読み、ファイルの追加の群の重みを数に入れ、差分更新をすべての枠で確かめるようにしました。
- テスト（`tests/regression.cpp`）：形式 1 のファイルのテスト（`evalform2`・`evalform2mob`・`evalbadconv`・`evalmobility`）は、形式 2 の `eval.dat` の Edax 4.5 の 13 群から形式 1 のファイルを作って使います。

64 枠の形（枠：使う像の数）：

| 形 | マス | 枠 |
|---|---|---|
| `c10`（Edax） | a5 a4 a3 a2 a1 b2 b1 c1 d1 e1 | 4 |
| `s100`（Edax） | b2 a1 b1 c1 d1 e1 f1 g1 h1 g2 | 4 |
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
| `cxd10`（48 枠には無い） | c1 d1 g1 h1 b2 d2 f2 g2 h2 f3 | 8 |
| `edw10`（48 枠には無い） | g5 f6 e7 f7 g7 d8 e8 f8 g8 h8 | 8 |

<details>
<summary>形の図（15 個）</summary>

形の図（X が形のマス、. が空きのマス。表のマスの並びのとおり 1 枠目の向き）：

`c10`（Edax）

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

`s100`（Edax）

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

`cxd10`（48 枠には無い）

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

`edw10`（48 枠には無い）

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

**強さ**

64 枠の版（refit、PGO の組）の対局です。Elo は新しい版から見た値で、かっこは 95% の幅です。

| 相手 | 条件 | 局数 | Elo |
|---|---|---|---|
| eval2（v4.5.5-newedax.12、standard） | 1 局 30 秒ずつ | 1,102 | +60.2（+47.9〜+72.6） |
| 11/05 の重み（64 枠、refit） | 1 局 30 秒ずつ | 2,204 | +11.2（+3.7〜+18.7） |
| 元の `eval.dat` の Edax | 両方 Lv18 | 4,378 | +77.5（+69.9〜+85.2） |
| Egaroucid（大会用の枝 `dev-lazysmp-ggs-b`） | 1 局 120 秒、8 スレッド | 180 | +23.2（+3.0〜+43.6） |
| 48 枠の版（refit） | 1 局 30 秒ずつ | 4,378 | +12.1（+6.9〜+17.3） |

- ProbCut：この重みで `refit` を `standard` と比べた結果、64 枠で `standard` は −1.0 Elo（−5.9〜+3.8、4,378 局）、48 枠で −7.7（−14.8〜−0.7、2,204 局）でした。

**互換性**

- 探索の結果は変わります（評価関数と ProbCut の既定）。`-probcut-model standard` で、前の ProbCut の形になります。
- book の形式は変わりません。前の版で学習した book は読めますが、その値は前の評価関数でつけたものです。
- `eval.dat`：それぞれの版の `eval.dat` は Release の ZIP（`bin/data/eval.dat`）にあり、git にはありません。リポジトリからビルドした版にも必要です。
- リポジトリの `bin/data` に残っている eval2 の `eval.dat` と、Edax 4.5 の `eval.dat` は、64 枠・48 枠の版には使えません（警告を出して読みますが、評価が正しくなりません）。
- 64 枠と 48 枠の `eval.dat` は同じ形式ですが、それぞれ自分の版のためのものです。
- libedax：API は v4.5.5-newedax.12 と同じです。ライブラリを同梱するプログラムは、同じ版の `eval.dat` も同梱してください。

**未確認**：Android の実機、Windows ARM64、book の速さ、Windows の MSVC と Android の NDK 以外でのビルド。

[検証記録](docs/validation/newedax.ja.md#v455-newedax13)

---

## v4.5.5-newedax.12

C 版と同じく、起動のときにほかのプログラムが開いていた book のファイルを、壊れたものとして置き換えることがなくなりました。見直しで見つけたほかの不具合も直し、動きを変えずにコードを整理しました。

**変更**

- C 版 v4.5.5-nikque.27 の変更を取り込みました（[C 版の修正一覧](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.ja.md#v455-nikque27)）。
  - 起動のときに開けない book のファイルは、2 秒ほど待って開き直します。それでも開けなければ、そのファイルには保存しません（`.damaged` も作りません）。
  - libedax に `edax_book_file_status` を足しました。
  - 動きを変えずにコードを整理しました。
- 評価のツール（`tools/eval`）：ファイルを開けないとき・メモリが足りないときの止め方を、同じ関数（`xfopen`・`xmalloc`・`xcalloc`・`die`）にまとめました（文と終了コードは同じ）。
- テスト：`tests/libedax_test.c`・`tests/book-analyze.cpp`・`tests/regression.cpp` に項目を足しました。
- マニュアル：4.1・6.6・7.3・8.4・8.5・12.2・12.4・13.1 に書きました（C 版と同じ文。12.2 のテストの項目数のほか）。

**不具合の修正**

- C 版 v4.5.5-nikque.27 と同じ直し：読めなかった book のファイルの印、`book import`、`book merge`、`book deviate` の仲間の `export`、`book moveinfo`、`book reducecount`、時間の `5:`、`config.ini` の場所、`weval` の表と統計、libedax など。
- 評価のツール：フォルダーの名前から作るファイルの名前が入れ物に収まらないとき、前は入れ物の外に書いていました。`sprepair`・`bookdump`・`w2dat2` は、落ちるか、壊れた名前で続けていました。今は `file name too long` と出して、終了コード 1 で止まります（10 本の 22 か所）。
- 評価のツール：`leafgen`・`w2dat2` は、メモリを確保できないときに `out of memory` で止まります。
- `tests/regression.cpp` の `evalmobility` は、前は移動力の重みを持つ newedax の `eval.dat` では必ず失敗していました。今は通ります（移動力の重みのない評価のファイルを、61 手の重みだけを書いて作ります）。

**互換性**

- 探索の結果・`eval.dat`・book のファイル形式は変わりません。
- 答えが v4.5.5-newedax.11 と違うのは、この版で直した場合だけです。book のエラーの文に入るソースの行番号は変わりました。
- libedax：設定の book のファイルを起動のときに開けなかったときは、どの保存もそのファイルを置き換えません（`edax_book_save_checked`・`edax_book_keep_unread_file` は 0）。book を自分で保存するプログラムは、初期化の後に `edax_book_file_status` を確かめてください。
- 評価のツールは、長すぎる名前のときのほかは、newedax.11 のツールと同じ入力で同じ出力です。

**未確認**：実 book での時間とメモリ、Android の実機、評価のツールのメモリが足りないときの動き（読んで確かめただけです）。

[検証記録](docs/validation/newedax.ja.md#v455-newedax12)

---

## v4.5.5-newedax.11

C 版と同じく、`book deviate`・`book deviate2`・`book deviate3` に `export` を足しました。book を広げずに、たどる手順をテキストファイルに書き出します。

**変更**

- C 版 v4.5.5-nikque.26 の変更を取り込みました（[C 版の修正一覧](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.ja.md#v455-nikque26)）。
  - `export <ファイル> [leaf|ends|all]` を足しました。
  - libedax に `edax_book_deviate_export` を足しました。
  - `@cancel id=N state=none export=1` の印を足しました。
- マニュアル：7.3 と 12.4 に書きました（C 版と同じ文）。

**互換性**

- `export` を付けないコマンドは v4.5.5-newedax.10 と同じです。
- 探索の結果・`eval.dat`・book のファイル形式は変わりません。

**未確認**：実 book での時間とメモリ、Android の実機。

[検証記録](docs/validation/newedax.ja.md#v455-newedax11)

---

## v4.5.5-newedax.10

C 版と同じく、うみがめ数の `exact=0` が付く値が、ビルドした環境によらず同じになるようにしました。

**変更**

- C 版 v4.5.5-nikque.25 の変更を取り込みました（[C 版の修正一覧](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.ja.md#v455-nikque25)）。
- マニュアル：12.4 に書きました（C 版と同じ文）。

**互換性**

- `exact=0` が付く値は、v4.5.5-newedax.9 と変わることがあります（どちらも上限です）。ほかの答えは同じです。
- 探索の結果・`eval.dat`・book のファイル形式は変わりません。

**未確認**：実 book での速さ、GUI での動き、Android の実機。

[検証記録](docs/validation/newedax.ja.md#v455-newedax10)

---

## v4.5.5-newedax.9

C 版と同じく、GUI の候補手の表のうみがめ数・局面の数と、`book levels`・`book missing` を前の局面の答えから引き継ぎ、同じ計算をし直さないようにしました。

**変更**

- C 版 v4.5.5-nikque.24 の変更を取り込みました（[C 版の修正一覧](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.ja.md#v455-nikque24)）。
  - うみがめ数：手の後の局面の網を数えるとき、その中の 1・2 手先の局面の値も覚えます。その局面が今の局面になったときに、網を作り直しません。
  - 局面の数（`part=count`）と `book levels`：局面ごとの答えを、book が変わるまで覚えます。
  - `book missing`：覚えている精度の段階から、聞いた範囲で何も見つからないとわかるときは、たどらずに 0 本と答えます。
- マニュアル：12.4 にこの 3 つを書きました（C 版と同じ文）。

**互換性**

- `book moveinfo`・`book levels`・`book missing` の答えは v4.5.5-newedax.8 と同じです。
- ただし、その局面だけから数えると `exact=0` が付くうみがめ数が、前の局面で覚えたちょうどの値（`exact=0` なし、より小さいか同じ値）になることがあります。
- 探索の結果・`eval.dat`・book のファイル形式は変わりません。

**未確認**：実 book での速さ、GUI での動き、Android の実機。

[検証記録](docs/validation/newedax.ja.md#v455-newedax9)

---

## v4.5.5-newedax.8

C 版と同じく、GUI の候補手の表の精度（`book moveinfo` の `stage=`）を前後の局面の答えから引き継ぎ、たどる量を減らしました。book のコマンドを `book` なしで入力したときに、入力の仕方を案内します。マニュアルと文書を書き直しました。

**変更**

- C 版 v4.5.5-nikque.23 の変更を取り込みました（[C 版の修正一覧](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.ja.md#v455-nikque23)）。
  - 覚えている今の局面の段階と、手の後の局面の先の局面の段階からわかる所は、たどらずに決めます。答え（`stage=`・`missing=`）は v4.5.5-newedax.7 と同じです。
  - 最上位で book のコマンドの名前（`leaf expand 30 30`・`fill2 2` など）を入力すると、警告に `(book command: type "book leaf expand 30 30" or "b leaf expand 30 30")` を添えます。
  - 最上位のコマンド・設定・`base` のコマンドと同じ名前には添えません。
- マニュアル：C 版と同じ見直しをしました。
  - 言い回しを見直しました。
  - book のコマンドを、全部 `book …` の形で書きました。
  - 9.4 の保存の時間を「18〜28 秒」にしました。
  - 4 章の `refit` の説明から、C 版の修正一覧の nikque.5 へリンクしました。
  - 12.4・13.2 に、この版の 2 つの変更を書きました。
- 文書：README と修正一覧を書き直し、テストと実測の記録を `docs/validation/` へ移し、`README-NIKQUE.*` を消しました。
- 配布版には `README-NIKQUE.*` の代わりに `README.md` が入ります。`config.ini` の初めの説明の行は、マニュアルの 4 章を指します。

**互換性**

- `book moveinfo` の答えは v4.5.5-newedax.7 と同じです。
- 知らないコマンドの警告の行は、book のコマンドの名前のときだけ、後ろに案内が付きます（前の部分は同じです）。
- 探索の結果・`eval.dat`・book のファイル形式は変わりません。

**未確認**：実 book での速さ、GUI での動き、Android の実機。

[検証記録](docs/validation/newedax.ja.md#v455-newedax8)

---

## v4.5.5-newedax.7

`book reduce`・`book reducecount` を、C 版と同じく速く、省メモリにしました。

**変更**

- C 版 v4.5.5-nikque.22 の変更を取り込みました（[C 版の修正一覧](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.ja.md#v455-nikque22)）。
  - `book reducecount` が速くなり、直後の `book reduce` は数えた結果を使い回します。削減後は使わなくなったメモリをすぐに解放します。
  - 新しい設定 `book-reduce-index`・`book-reduce-reuse`・`book-reduce-shrink`・`book-reduce-walk` を追加しました（どれも既定のままで使えます）。
- コマンドの答えは C 版 v4.5.5-nikque.22 と同じです。

**互換性**

- 残る局面と局面ごとの中身は v4.5.5-newedax.6 と同じです。
- 探索の結果・`eval.dat`・book のファイル形式は変わりません。

**未確認**：newedax での速さ（29 GB の実 book での `book reducecount` の速さは C 版でだけ測っています）、Android の実機。

[検証記録](docs/validation/newedax.ja.md#v455-newedax7)

---

## v4.5.5-newedax.6

`book reduce`・`book reducecount` に、C 版と同じ不具合の修正と組み直しを入れました。

**変更**

- C 版 v4.5.5-nikque.21 の変更を取り込みました（[C 版の修正一覧](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.ja.md#v455-nikque21)）。
  - `book reducecount` が `n-tasks` のスレッド数で並列に動くようになりました。
  - 壊れた book で `book reducecount` が終わらない不具合と、誤った入力でも局面を消していた不具合を直しました（今はエラーまたは警告を出し、book を変えません）。
- コマンドの答えは C 版 v4.5.5-nikque.21 と同じです。

**互換性**

- 正常な book では、残る局面と `book reduce` の後に保存する book は v4.5.5-newedax.5 と同じです。
- 探索の結果・`eval.dat`・book のファイル形式は変わりません。

**未確認**：29 GB の実 book、Android の実機。

[検証記録](docs/validation/newedax.ja.md#v455-newedax6)

---

## v4.5.5-newedax.5

book を小さくするコマンド `book reduce` と、事前に数を確かめる `book reducecount` を追加しました。

**変更**

- C 版 v4.5.5-nikque.20 の変更を取り込みました（[C 版の修正一覧](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.ja.md#v455-nikque20)）。
  - `book reduce X Y` は、`book deviate3 X Y` がたどる範囲の局面だけを残し、ほかを消します。`book reducecount` は、残る数・消える数を数えます（book は変えません）。
- コマンドと答えは C 版 v4.5.5-nikque.20・v4.5.5-eval2.8 と同じです。
- 使い方：[マニュアル 8.4](manual/ja/08-book-maintenance.md)・[12.4](manual/ja/12-integration.md)

**互換性**：探索の結果・`eval.dat`・book のファイル形式は v4.5.5-newedax.4 と変わりません。

**未確認**：新しいコマンドの速さ、29 GB の実 book、Android の実機。

[検証記録](docs/validation/newedax.ja.md#v455-newedax5)

---

## v4.5.5-newedax.4

GUI のプログラム向けに、book を解析するコマンドを追加しました。

**変更**

- C 版 v4.5.5-nikque.19 の変更を取り込みました（[C 版の修正一覧](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.ja.md#v455-nikque19)）。
  - 新しいコマンド：`book moveinfo`・`book missing`・`book levels`・`book bestlines`・`book findline`、`stop id=N`。
- コマンドと答えは C 版 v4.5.5-nikque.19・v4.5.5-eval2.7 と同じです。
- 新しいコマンドは C 版より遅いものがありません（1,914 万局面の book、1 スレッドで、初手からの局面の数は C 版の 5.92 秒に対して 5.03 秒）。book だけの作業メモリは C 版の 1,235 MB に対して 880 MB です。
- Windows の実行ファイルのビルド方法は v4.5.5-newedax.3 と同じです（`wEdax-x86-64-v4.exe` は PGO なし、ほかの 4 種は PGO）。
- 使い方：[マニュアル 12.4](manual/ja/12-integration.md)

**互換性**：探索の結果・`eval.dat`・book のファイル形式は v4.5.5-newedax.3 と変わりません。

**未確認**：29 GB の実 book、Android の実機。

[検証記録](docs/validation/newedax.ja.md#v455-newedax4)

---

## v4.5.5-newedax.3

Windows の実行ファイルのビルド方法を変えて、1 スレッドの速さを C 版に合わせました。プレリリースではない、最初の正式版です。

**変更**

- Windows の実行ファイルは、`wEdax-x86-64-v4.exe` が PGO（実行の記録を使う最適化）なし、ほかの 4 種が PGO でビルドしたものになりました（`tools/port/build-release.cmd`）。v4.5.5-newedax.2 はすべて PGO なしでした。
- `wEdax-x86-64-v4.exe` は PGO なしの方が速いため、v4.5.5-newedax.2 と同じ機械語のままです。DLL 3 種と Android も PGO なしで、機械語は変わりません。
- 速さ（C 版 v4.5.5-eval2.6 との比）：1 スレッドの中盤で、`wEdax-x86-64-v4.exe` は C 版より約 3〜4% 速くなっています（0.961〜0.969）。`wEdax-x86-64.exe`・`wEdax-x86-sse.exe`・`wEdax-x86.exe` は C 版とほぼ同じか速いです。
- **`wEdax-x86-64-v3.exe` は C 版より約 1% 遅いままです**（1 スレッドの中盤、32 スレッドの `book learn`・`fill` 系）。AVX-512 の CPU では `wEdax-x86-64-v4.exe` を使ってください。
- 最大メモリは C 版と同じか少なくなっています（`book import` 614 → 545 MiB）。C 版が作った 3,473 万局面の book の読み込みでは、最大メモリが 3,243 → 1,667 MiB です。
- eval2 のツールのうち `w2dat2`・`label` は、C のツールより約 0.7% 遅いです。ほかは同じか速いです。
- 計測の記録：`docs/port/MEASURE-2.md`。

**互換性**

- 探索の結果は v4.5.5-newedax.2 と同じです（v4.5.5-eval2.6 とも同じ）。
- `eval.dat`・book のファイル形式は変わりません。

**注意**：PGO は、ビルドのたびに機械語が少し変わります。配布する `-v3`・`wEdax-x86-sse.exe`・`wEdax-x86.exe` は、配布するものそのものを C 版と比べ直してあります。

**未確認**：`wEdax-x86-64-v3.exe` と `w2dat2`・`label` が遅い原因、Android の実機。

[検証記録](docs/validation/newedax.ja.md#v455-newedax3)

---

## v4.5.5-newedax.2

v4.5.5-newedax.1 のクロスチェックで見つかった不具合を直しました。1 スレッドで C 版より遅い実行ファイルが残るため、まだプレリリースです。

**変更**

- 実行ファイルとライブラリに、ビルドしたときのフォルダーが入らなくなりました（C 版 v4.5.5-nikque.18 と同じ直し。[C 版の修正一覧](https://github.com/Nikque/edax-reversi-AVX/blob/edax-4.5.5-fixes/RELEASE-NOTES.ja.md#v455-nikque18)）。エラーの文言はソースのファイル名だけになります。
- ZIP の `tools/eval` に `vcomp140.dll`（MSVC の OpenMP の実行時ライブラリ）を入れました。7 つのツールが使います。
- ツールのスクリプトは、入力のフォルダーを引数で受け取ります（`--ev`・`--book40`・`-Inputs`。既定値なし）。
- Windows の実行ファイルは、v4.5.5-newedax.1 と同じく PGO なしです。
- 速さ（32 スレッドの `wEdax-x86-64-v4.exe`）：v4.5.5-newedax.1 と同じで、C 版 v4.5.5-eval2.6 と同じか速いです（book のコマンド 7 種で 0.959〜1.001 倍の時間）。最大メモリは同じか少ないです。

**不具合の修正**（eval2 のツールと `-eval-convert`）

- `bookdump`・`bookscan`：石が 4 つ未満の局面がある book で、手数が負になっていました（C のツールにも同じ不具合）。そのような book は受け付けません。
- ツールの設定で同じスカラーを 2 回書くと 2 回数えていました。パターンに同じマスが 2 つあると、対称の置き換えが壊れていました。どちらも受け付けません。
- 形式 2 の 5 バイトの数で、5 バイト目の上位 4 ビットを見ていませんでした（2^32−1 は読み、2^32 は受け付けません）。
- `build-evtools.ps1` は、ビルドできなかったツールがあっても終了コード 0 でした。1 を返します。
- `-eval-convert`：54 手数より少ない形式 1 のファイルを変換し、どちらの形式でも読めないファイルを作っていました。形式 2 で手数×重みの数が 32 ビットに入らないとあふれていました。どちらも受け付けません。

**互換性**：探索の結果は v4.5.5-newedax.1 と同じです（v4.5.5-eval2.6 とも同じ）。

**注意**：1 スレッドでは、`wEdax-x86-64-v4.exe` 以外の 4 種が C 版（PGO のビルド）より遅いです（中盤で最大 11.6%、`wEdax-x86-sse.exe`）。v4.5.5-newedax.3 で直しています。

**未確認**：既定のビルド（eval2）の強さ、Windows の MSVC と Android の NDK 以外のビルド、Android の実機、AddressSanitizer・ThreadSanitizer、数十億局面の実 book、別の人による読み直し。

[検証記録](docs/validation/newedax.ja.md#v455-newedax2)

---

## v4.5.5-newedax.1

new edax の最初の版です。C 版 v4.5.5-nikque.17 を C++23 に移植し、eval2 の系列（v4.5.5-eval2.5）の評価関数を既定にしました。別の人によるソースの確認と、既定のビルドの速さの計測がまだのため、プレリリースです。

**変更**

- 既定のビルドは、ビルドの選択肢 `EVAL_MOBILITY` が 1、`bin/data/eval.dat` が eval2 のものです。
- コマンドと選択肢は C 版と同じです。
- book と評価ファイルの形式 2（`NEDXBOOK`・`NEDXEVAL`）を追加しました。book は約 33% 小さくなり、CRC-32C がつきます。形式 1 も読めます。
- 形式を変えるオプション：

| オプション | 働き |
|---|---|
| `-book-format` | book のファイル形式を選ぶ |
| `-book-convert` | book の形式を変換する（両方向） |
| `-eval-convert` | 評価ファイルの形式を変換する（両方向） |

- book の局面がメモリ上で 40 バイトになりました（C 版は 48 バイト）。649 万局面の book の読み込みで、メモリが 11〜45% 少なくなります。
- 新しいビルドの選択肢：`EVAL_MOBILITY=0`（着手可能数の重みを読まない）、`EVAL_SLOTS=64`（64 枠。より強い評価関数の土台。既定は 48）。
- `tools/eval`：eval2 の評価関数の学習ツールの C++ 版（14 本。ZIP には x64 AVX-512 用だけ）。
- 速さ：C 版 v4.5.5-nikque.17 と同じビルドの指定（PGO なし）で比べて、C 版より遅い条件は見つかりませんでした（`docs/port/MEASURE-1.md`）。
- 配布版（MSVC 2022 `/O2 /GL`、**PGO なし**。Android は NDK r27d）：
  - Windows：`wEdax-x86-64-v4.exe`・`wEdax-x86-64-v3.exe`・`wEdax-x86-64.exe`・`wEdax-x86-sse.exe`・`wEdax-x86.exe`、`libedax-x64-v4.dll`・`libedax-x64-v3.dll`・`libedax-x64.dll`。
  - Android：`aEdax-arm64-v8a`・`aEdax-armeabi-v7a`・`libedax-arm64-v8a.so`・`libedax-armeabi-v7a.so`。
  - Linux・macOS・Windows ARM64 の実行ファイルはありません（ソースからのビルドは README の「Build」）。
  - PGO なしにしたのは、`book deviate` 系が PGO なしの方が約 1.5〜2% 速かったためです（C 版でも同じ）。ほかの条件では、PGO のあり・なしを直接は比べていません。

**互換性**

- 探索の結果は C 版と同じです。eval2 の `eval.dat` で v4.5.5-eval2.5 と、Edax 4.5 の `eval.dat` で v4.5.5-nikque.17 と同じです。
- 形式 1 の book と `eval.dat` はそのまま読めます。

**未確認**：既定のビルド（eval2）の速さと強さ、Windows の MSVC と Android の NDK 以外のビルド、Android の実機、メモリの誤りを検出するビルドでのテスト、数十億局面の実 book、別の人によるソースの確認。

[検証記録](docs/validation/newedax.ja.md#v455-newedax1)
