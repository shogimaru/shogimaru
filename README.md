# 将棋丸

[![ActionsCI](https://github.com/shogimaru/shogimaru/actions/workflows/actions.yml/badge.svg)](https://github.com/shogimaru/shogimaru/actions/workflows/actions.yml)
[![License](https://img.shields.io/badge/license-MIT-blue)](https://opensource.org/licenses/MIT)
[![Release](https://img.shields.io/github/v/release/shogimaru/shogimaru.svg)](https://github.com/shogimaru/shogimaru/releases)

ブラウザで本格将棋が指せることを目指して、Qt/WebAssemblyで将棋GUIを作っています。
エンジンにはやねうら王を使用しています。

## 開発環境

 - WebAssembly
   - [Emscripten](https://emscripten.org/)
   - [Qt for WebAssembly (multithread)](https://www.qt.io/)  
     Qt Company が提供しているWebAssembly向けバイナリはシングルスレッド版です（2022年1月現在）。マルチスレッド版はソースからビルドする必要があります。

 - デスクトップ版
   - コンパイラ （gccやclangなど）
   - [Qt version 6 for Desktop](https://www.qt.io/)

## ビルド

1. クローン
 ```
 $ git clone --recursive https://github.com/shogimaru/shogimaru.git
 ```

2. 将棋エンジン（やねうら王）をコンパイル  
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

コンピュータの設定はこんな感じ:  
  Skill : Rating  
  20 :   3000  最強  
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

- 初期式  新R=旧R+((相手R-旧R)±400)/(N+1)   N:通算対局数  
- 通常式  新R=旧R+((相手R-旧R)±400)/25 （今までと同じ計算式）

25局目以降:
 勝ったらプラス(+1〜+31)
 負けたらマイナス(-31〜-1)


## ウェブサイト

 将棋丸 - WebAssembly
 https://shogimaru.com/

### Twitter
 https://twitter.com/shogimaru
