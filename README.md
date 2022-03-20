# 将棋丸

[![ActionsCI](https://github.com/shogimaru/shogimaru/actions/workflows/actions.yml/badge.svg)](https://github.com/shogimaru/shogimaru/actions/workflows/actions.yml)
[![License](https://img.shields.io/badge/license-MIT-blue)](https://opensource.org/licenses/MIT)
[![Release](https://img.shields.io/github/v/release/shogimaru/shogimaru.svg)](https://github.com/shogimaru/shogimaru/releases)

ブラウザでも本格的な将棋が指せることを目指して、オープンソースの将棋GUIを作っています。
将棋思考エンジンにはやねうら王、UIにはQt/WebAssemblyを使用しています。  
棋譜の解析モードも実装されています。

## 実行
デスクトップ版では将棋思考エンジンを設定する必要があります。将棋思考エンジンは2つ以上設定することができます。

 - WebAssembly
   ブラウザで所定のページを開き、将棋丸を起動します。
   将棋思考エンジンは組み込まれているので（新たにエンジンを追加することはできません）、[対局] ボタンをクリックして対局を開始することができます。

 - デスクトップ版
   将棋思考エンジンを設定するために、次の手順を実施します。
   1. 将棋思考エンジンをダウンロードし、フォルダへ展開しておきます。
   2. 将棋丸の実行ファイル(shogimaru.exe)をダブルクリックし起動します。
   3. 将棋思考エンジンを追加するため、[設定] メニューをクリックし、設定画面を開きます。
   4. [追加] ボタンをクリックし、1.で展開した将棋思考エンジンの実行ファイル(exe)を選択します。
   5. 設定可能なオプションの一覧が表示されるので、必要に応じてオプションの値を変更します。初期値のままで動作することがありますが、エラーになる場合は定跡ファイルの格納ディレクトリ（BookDir）や評価関数用ファイルの格納ディレクトリ（EvalDir）などを正しく設定します。
   6. 設定画面を閉じた後、[対局] ボタンをクリックして対局を開始します。

## 開発環境
ソースファイルをビルドして実行ファイルを作成することができます。次のとおり開発環境を用意します。

 - WebAssembly
   - [Emscripten](https://emscripten.org/)
   - [Qt for WebAssembly (multithread)](https://www.qt.io/)  
     Qt Company が提供しているWebAssembly向けバイナリはシングルスレッド版です（2022年1月現在）。マルチスレッド版はソースからビルドする必要があります。

 - デスクトップ版 - Windows, macOS, Linux
   - コンパイラ （gccやclangなど）
   - [Qt version 6 for Desktop](https://www.qt.io/)

### ビルド

1. クローン
 ```
 $ git clone --recursive https://github.com/shogimaru/shogimaru.git
 ```

2. 将棋思考エンジン（やねうら王）をコンパイル  
 Makefileは適宜修正してください。
 ```
 $ cd engines/YaneuraOu/source/
 $ make
 ```

3. 将棋丸をコンパイル
 - WebAssembly  
 Emscripten(2.0.6)環境を有効にした上で次のコマンドを実行する。
 ```
 $ qmake -spec wasm-emscripten CONFIG+=release && make
 ```

 - デスクトップ版 (Qt6/Qt5)  
 開発するときはこちらが便利です。
 ```
 $ qmake CONFIG+=release && make
 
 (clang の場合)
 $ qmake -spec linux-clang CONFIG+=release && make
 ```

### レーティング戦

将棋思考エンジンにSkillLevelオプションがある場合（やねうら王など）、次のとおり設定されます:  
  SkillLevel : Rating  
  20 :   3000  最強（手加減なし）  
  18 :   2800  
  16 :   2600  
  14 :   2400  
  12 :   2200  
  10 :   2000  
   8 :   1800  
   6 :   1600  
   4 :   1400  
   2 :   1200  
   0 :   1000  最弱  

#### 対局終了後のレーティング計算

- 初期式:  新R = 旧R + ((相手R - 旧R)±400) / (N+1)  
- 通常式:  新R = 旧R + ((相手R - 旧R)±400) / 25  
N: 通算対局数  

25局目以降:
 勝ったらプラス(+1〜+31)
 負けたらマイナス(-31〜-1)


## ウェブサイト

 将棋丸 - WebAssembly
 https://shogimaru.com/

### Twitter
 https://twitter.com/shogimaru
