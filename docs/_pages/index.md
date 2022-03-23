---
permalink: /
title: "将棋丸"
excerpt: "将棋丸はオープンソースの将棋GUIです。WebAssemblyで実装されたWeb版もあります。"
last_modified_at: 2022-03-23
toc: true
---

[![ActionsCI](https://github.com/shogimaru/shogimaru/actions/workflows/actions.yml/badge.svg)](https://github.com/shogimaru/shogimaru/actions/workflows/actions.yml)
[![License](https://img.shields.io/badge/license-MIT-blue)](https://opensource.org/licenses/MIT)
[![Release](https://img.shields.io/github/v/release/shogimaru/shogimaru.svg)](https://github.com/shogimaru/shogimaru/releases)

将棋丸はオープンソースの将棋 GUI です。  
次のような特徴があります。

- クロスプラットフォーム - Windows, macOS, Linux, WebAssembly
- 多くの将棋思考エンジンに対応
- MultiPV 対応
- 棋譜の解析モード
- 棋譜読込・保存

[ダウンロード]({{ "https://github.com/shogimaru/shogimaru/releases" }}){: .btn .btn--success .btn--medium}
[将棋丸ブラウザ版]({{ "https://shogimaru.com" }}){: .btn .btn--info .btn--medium}

**※** 将棋を指すには **[将棋思考エンジン](#将棋思考エンジン)** が別途必要です。
{: .notice--warning}

## 実行

デスクトップ版では将棋思考エンジンを手動で設定する必要があります。将棋思考エンジンは 2 つ以上設定することができます。

- WebAssembly
  ブラウザで所定のページを開き、将棋丸を起動します。
  将棋思考エンジンは組み込まれているので（新たにエンジンを追加することはできません）、[対局] ボタンをクリックして対局を開始することができます。

- デスクトップ版
  将棋思考エンジンを設定するために、次の手順を実施します。
  1.  将棋思考エンジンをダウンロードし、フォルダへ展開しておきます。
  2.  将棋丸の実行ファイル(shogimaru.exe)をダブルクリックし起動します。
  3.  将棋思考エンジンを追加するため、[設定] メニューをクリックし、設定画面を開きます。
  4.  [追加] ボタンをクリックし、1.で展開した将棋思考エンジンの実行ファイル(exe)を選択します。
  5.  設定可能なオプションの一覧が表示されるので、必要に応じてオプションの値を変更します。初期値のままで動作することがありますが、エラーになる場合は定跡ファイルの格納ディレクトリ（BookDir）や評価関数用ファイルの格納ディレクトリ（EvalDir）などを正しく設定します。
  6.  設定画面を閉じた後、[対局] ボタンをクリックして対局を開始します。

### レーティング戦

将棋思考エンジンに SkillLevel オプションがある場合（やねうら王など）、次のとおり自動的に値が設定されます。

| SkillLevel | Rating                  |
| ---------- | ----------------------- |
| 20         | 3000 最強（手加減なし） |
| 18         | 2800                    |
| 16         | 2600                    |
| 14         | 2400                    |
| 12         | 2200                    |
| 10         | 2000                    |
| 8          | 1800                    |
| 6          | 1600                    |
| 4          | 1400                    |
| 2          | 1200                    |
| 0          | 1000 最弱               |

#### 対局終了後のレーティング計算

- 初期式: 新 R = 旧 R + ((相手 R - 旧 R)±400) / (N+1)
- 通常式: 新 R = 旧 R + ((相手 R - 旧 R)±400) / 25  
  N: 通算対局数

25 局目以降:
勝ったらプラス(+1〜+31)
負けたらマイナス(-31〜-1)

### 開発

将棋丸はオープンソースであり [GitHub](https://github.com/shogimaru/shogimaru) で公開されています。

### 将棋思考エンジン

将棋ソフトの開発者にとって「いかにソフトを強くするか」が重要であり、駒移動や盤面表示などの GUI は本質的な部分ではありません。強さを追求したい開発者にとって、GUI も作成するのはかなり面倒なことので、将棋ソフトは思考エンジン（アルゴリズム）と GUI の２つのプログラムに分離されました。

この２つのプログラムは USI プロトコル（Universal Shogi Interface）という通信規約でやりとりすることになっており、この規約が実装されている思考エンジンとGUIとを組み合わせることで将棋ソフトとして使用することができます。

将棋思考エンジンのダウンロードリンクを集めてみました（順不同）

| エンジン名                                                              | ダウンロード                                                                                                             | 備考                                                                         |
| ----------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------ | ---------------------------------------------------------------------------- |
| [やねうら王](https://yaneuraou.yaneu.com/)／ふかうら王／水匠            | [リリースページ](https://github.com/yaneurao/YaneuraOu/releases)                                                         | 実行ファイル詰め合わせ EXE ファイルによって CUDA, cuDNN, TensorRT なども必要 |
| [Apery](https://hiraokatakuya.github.io/apery/)                         | [リリースページ](https://github.com/HiraokaTakuya/apery/releases)                                                        |                                                                              |
| [elmo](https://mk-takizawa.github.io/elmo/howtouse_elmo.html)           | [elmo.shogi.zip](https://drive.google.com/file/d/0B0XpI3oPiCmFalVGclpIZjBmdGs/edit?resourcekey=0-qNCo0QeQN9ZMFRa7_r90zw) | 定跡ファイルや評価関数ファイルをやねうら王に設定する                         |
| [dlshogi](https://github.com/TadaoYamaoka/DeepLearningShogi)            | [リリースページ](https://github.com/TadaoYamaoka/DeepLearningShogi/releases)                                             | ディープラーニング系                                                         |
| [技巧](https://github.com/gikou-official/Gikou)                         | [リリースページ](https://github.com/gikou-official/Gikou/releases)                                                       |
| [芝浦将棋 Softmax](https://github.com/tanuki12hiromasa/ShogiStudyThird) | [リリースページ](https://github.com/tanuki12hiromasa/ShogiStudyThird/releases/tag/210331)                                | 他にもファイルを置かないといけないみたい                                     |
| [白ビール](https://github.com/Tama4649/Kristallweizen)                  |                                                                                                                          | 評価関数ファイルをやねうら王に設定する                                       |
| [たぬきち](https://github.com/nodchip/tanuki-)                          | [リリースページ](https://github.com/nodchip/tanuki-/releases)                                                            | やねうら王ベース                                                             |
| [GPS 将棋](https://gps.tanaka.ecc.u-tokyo.ac.jp/gpsshogi/)              | [ダウンロードページ](https://gps.tanaka.ecc.u-tokyo.ac.jp/gpsshogi/index.php?%A5%C0%A5%A6%A5%F3%A5%ED%A1%BC%A5%C9)       | もう更新されていない？                                                       |
