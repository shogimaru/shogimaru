---
permalink: /
title: "将棋丸"
excerpt: "将棋丸はオープンソースの将棋GUIです。デスクトップ版アプリとWebAssemblyで実装されたブラウザ版があります。将棋丸はブラウザ将棋で最強クラスのソフトです。"
last_modified_at: 2022-03-23
toc: true
---

[![Release](https://img.shields.io/github/v/release/shogimaru/shogimaru.svg)](https://github.com/shogimaru/shogimaru/releases)
[![License](https://img.shields.io/badge/license-MIT-blue)](https://opensource.org/licenses/MIT)

将棋丸はオープンソースの将棋 GUI です。  
将棋思考エンジンと通信するための USI プロトコルに対応しており、次の特徴があります。

- クロスプラットフォーム - Windows, Mac, Linux, WebAssembly
- 棋譜の解析モード、評価グラフ
- 複数の読み筋を表示（MultiPV対応）
- CSA形式棋譜の読込・保存（インターネット上の棋譜も読込可能）
- 多言語対応（日本語、英語）

[<i class="fas fa-cloud-download-alt"></i> ダウンロード]({{ "https://github.com/shogimaru/shogimaru/releases" }}){: .btn .btn--success .btn--medium}
[将棋丸ブラウザ版 <i class="fas fa-arrow-right"></i>]({{ "https://shogimaru.com" }}){: .btn .btn--info .btn--medium}

ダウンロードページには Windows 向け（zip）と Mac 向け（dmg）のアプリが置いてあります。

デスクトップ版では **[将棋思考エンジン](#-将棋思考エンジン)** が別途必要です。[こちら](#-デスクトップ版)を参考にセットアップしてください。
{: .notice--warning}

ブラウザ版は PC のブラウザ（Chrome や Firefox など）で動作します。2024 年 12 月時点でスマートフォンでは正常に動作しません。
今後対応予定です。
{: .notice--info}

<img width="600" src="{{ site.baseurl }}/assets/images/shogimaru-main.png" alt="将棋丸メイン画面">

## <i class="far fa-paper-plane"></i> 実行

### <i class="fab fa-mixcloud"></i> ブラウザ版（WebAssembly）

ネットで手軽に遊べるブラウザ将棋です。どんな OS でもブラウザさえあれば遊べます。

1. [将棋丸ブラウザ版](https://shogimaru.com) へアクセスし、将棋丸を起動します。
2. 将棋思考エンジンは組み込まれているので（新たにエンジンを追加できません）、`対局`ボタンをクリックしてすぐに対局を開始することができます。

計算処理スピードはデスクトップ版に劣りますがとっても強いです:laughing:

### <i class="fas fa-desktop"></i> デスクトップ版

Windows、Mac、Linux などで動作する GUI アプリです。CPU/GPU をフルに活用し計算スピードを高めるためにはデスクトップ版を使います。別途、将棋思考エンジンが必要です。

Windows 向け将棋丸は次の手順を参考にセットアップしてください。

1.  [ダウンロードページ](https://github.com/shogimaru/shogimaru/releases)から Zip ファイルをダウンロードし、展開します。
2.  将棋丸の実行ファイル shogimaru.exe をダブルクリックし起動します。
3.  将棋思考エンジンを追加するため、`設定`メニューをクリックし、設定画面を開きます。
4.  `追加`ボタンをクリックし、別でインストールした将棋思考エンジンの実行ファイル(exe)を選択します。
5.  設定可能なオプションの一覧が表示されるので、必要に応じてオプションの値を変更します。初期値のままで動作することがありますが、エラーになる場合は定跡ファイルの格納ディレクトリ（BookDir）や評価関数用ファイルの格納ディレクトリ（EvalDir）などを正しく設定します。
6.  設定画面を閉じた後、`対局`ボタンをクリックして対局を開始します。

将棋思考エンジンは2つ以上登録でき、適宜切り替えて対局や棋譜解析することができます。

### レーティング対局

最近の思考エンジンは人間が全く敵わないほど強くなっているので、レーティング対局では手加減するよう設定しました。

将棋思考エンジンに SkillLevel オプションがある場合（やねうら王など）、対局の際に次の値が自動的に設定されます。

| Rating | SkillLevel            |
| ------ | --------------------- |
| 3000   | 20 最強（手加減なし） |
| 2800   | 18                    |
| 2600   | 16                    |
| 2400   | 14                    |
| 2200   | 12                    |
| 2000   | 10                    |
| 1800   | 8                     |
| 1600   | 6                     |
| 1400   | 4                     |
| 1200   | 2                     |
| 1000   | 0 最弱                |

最弱でも十分に強いし...

#### 対局終了後のレーティング計算

初期式: `新R = 旧R + ((相手R - 旧R) ± 400) / (N + 1)`  
通常式: `新R = 旧R + ((相手R - 旧R) ± 400) / 25`  
 N: 通算対局数

25 局目以降:  
勝ったらプラス(+1〜+31)  
負けたらマイナス(-31〜-1)

## <i class="fas fa-laptop-code"></i> 開発

将棋丸はオープンソースであり [GitHub](https://github.com/shogimaru/shogimaru) で公開されています。ビルド方法などはそちらをご覧ください。 
要望やバグ報告などお待ちしています。

## <i class="fas fa-brain"></i> 将棋思考エンジン

将棋ソフトの開発者にとって「いかにソフトを強くするか」に興味があり、駒移動や盤面表示などの GUI はあまり本質的な部分とは考えられていません。強さを追究したい開発者にとって、GUI も作成するのはかなり面倒なことなので、将棋ソフトは思考エンジン（アルゴリズム）と GUI の２つのプログラムに分離されました。

この２つのプログラムは USI プロトコルという通信規約でやりとりすることになっており、この規約が実装されている思考エンジンと GUI とを組み合わせることにより将棋ソフトとして使用することができます。

思考エンジンにとって 、局面を評価し数値化する「評価関数」の出来ばえが強さに大きく影響するものであり、開発者が注力している部分です。やねうら王などの思考エンジンではこの部分（評価関数ファイルと呼ぶ）を差し替えることができます。

思考エンジンのダウンロードリンクを集めてみました（順不同）。この他にも思考エンジンは作られています。

| 思考エンジン                                                            | ダウンロード                                                                                                             | 備考                                                                          |
| ----------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------ | ----------------------------------------------------------------------------- |
| [やねうら王](https://yaneuraou.yaneu.com/)／ふかうら王／水匠            | [リリースページ](https://github.com/yaneurao/YaneuraOu/releases)                                                         | 実行ファイル詰め合わせ。EXE ファイルによって CUDA, cuDNN, TensorRT なども必要 |
| [dlshogi](https://github.com/TadaoYamaoka/DeepLearningShogi)            | [リリースページ](https://github.com/TadaoYamaoka/DeepLearningShogi/releases)                                             | ディープラーニング系                                                          |
| [たぬきち](https://github.com/nodchip/tanuki-)                          | [リリースページ](https://github.com/nodchip/tanuki-/releases)                                                            | やねうら王ベース                                                              |
| [Apery](https://hiraokatakuya.github.io/apery/)                         | [リリースページ](https://github.com/HiraokaTakuya/apery/releases)                                                        |                                                                               |
| [技巧](https://github.com/gikou-official/Gikou)                         | [リリースページ](https://github.com/gikou-official/Gikou/releases)                                                       |
| [芝浦将棋 Softmax](https://github.com/tanuki12hiromasa/ShogiStudyThird) | [リリースページ](https://github.com/tanuki12hiromasa/ShogiStudyThird/releases/tag/210331)                                | 他にも評価関数ファイルを置かないといけないみたい                              |
| [白ビール](https://github.com/Tama4649/Kristallweizen)                  |                                                                                                                          | 評価関数ファイルをやねうら王に設定する                                        |
| [elmo](https://mk-takizawa.github.io/elmo/howtouse_elmo.html)           | [elmo.shogi.zip](https://drive.google.com/file/d/0B0XpI3oPiCmFalVGclpIZjBmdGs/edit?resourcekey=0-qNCo0QeQN9ZMFRa7_r90zw) | 定跡ファイルや評価関数ファイルをやねうら王に設定する                          |
| [GPS 将棋](https://gps.tanaka.ecc.u-tokyo.ac.jp/gpsshogi/)              | [ダウンロードページ](https://gps.tanaka.ecc.u-tokyo.ac.jp/gpsshogi/index.php?%A5%C0%A5%A6%A5%F3%A5%ED%A1%BC%A5%C9)       | もう更新されていない？                                                        |

## <i class="fas fa-mouse"></i> 将棋 GUI

ネット検索して見つけられた、将棋丸と同じ USI 対応の将棋 GUI のサイトをまとめました。

| GUI                                                              | 備考                                                                                                                     |
| ---------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------ |
| [将棋所](http://shogidokoro.starfree.jp/)                        | 広く使われている GUI                                                                                                     |
| [ShogiGUI](http://shogigui.siganus.com/)                         | 棋譜解析にとても便利                                                                                                     |
| [ShogiHome](https://sunfish-shogi.github.io/shogihome/) | [Electron](https://www.electronjs.org/) で作られたオープンソース将棋アプリ。ブラウザ版もある。 |
| [WhaleWatcher](http://garnet-alice.net/programs/whalewatcher/)   | クジラちゃんに癒やされながら指せます                                                                                     |
| [ShogiDroid](http://shogidroid.siganus.com/)                     | Android 用                                                                                                               |

将棋丸はこれらのソフトを参考にしています。

## <i class="fab fa-chrome"></i> ブラウザ将棋

インストールが面倒な方に最適！ブラウザさえあれば将棋が指せるソフト「ブラウザ将棋」をまとめました。
ブラウザ将棋は初心者向けのものが多い印象で、その中でも最新の思考エンジンが実装されている[将棋丸](https://shogimaru.com/index.en.html)が最強か？

| サイト                                                                   | 備考                                                       |
| ------------------------------------------------------------------------ | ---------------------------------------------------------- |
| [ぴよ将棋 w](https://www.studiok-i.net/ps/)                              | ぴよ将棋のブラウザ版で思考エンジンはサーバサイドにあります |
| [きのあ将棋](https://syougi.qinoa.com/ja/game/)                          | 個性豊かなキャラクタと対局できるブラウザ将棋               |
| [つぼ将棋](https://www.afsgames.com/shogi.htm)                           | オンライン対戦もできる初心者向けのブラウザ将棋             |
| [将皇ブラウザ版](https://ken1shogi.sakura.ne.jp/shogiwebgl/)             | 将皇が WebAssembly で実装されているもよう                  |
| [Web ブラウザ将棋](https://www.programmingmat.jp/webgame_lib/sg99a.html) | JavaScript によるブラウザ将棋                              |
| [こまお](http://usapyon.game.coocan.jp/komao/)                           | 初心者用ブラウザ将棋                                       |

## <i class="fas fa-mobile-alt"></i> 将棋アプリ

スマートフォンで遊べるのは便利。ストアで探せば見つかるのでここでは割愛。将棋丸もいずれはアプリに。
