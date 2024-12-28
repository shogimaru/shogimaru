# 将棋丸

[![ActionsCI](https://github.com/shogimaru/shogimaru/actions/workflows/actions.yml/badge.svg)](https://github.com/shogimaru/shogimaru/actions/workflows/actions.yml)
[![License](https://img.shields.io/badge/license-MIT-blue)](https://opensource.org/licenses/MIT)
[![Release](https://img.shields.io/github/v/release/shogimaru/shogimaru.svg)](https://github.com/shogimaru/shogimaru/releases)

オープンソースの将棋 GUI を C++ で作っています。
思考エンジンと通信するための USI プロトコルに対応しており、次の特徴があります。

- クロスプラットフォーム - Windows, Mac, Linux, WebAssembly
- 棋譜の解析モード、評価グラフ
- 複数の読み筋を表示（MultiPV対応）
- CSA形式棋譜の読込・保存（インターネット上の棋譜も読込可能）
- 多言語対応（日本語、英語）

## 開発環境

ソースファイルをビルドして実行ファイルを作成するために、次のとおり開発環境を用意します。

- WebAssembly 版

  - [Emscripten](https://emscripten.org/)
  - [Qt for WebAssembly (multithread)](https://www.qt.io/)  

- デスクトップ版 - Windows, macOS, Linux
  - コンパイラ （gcc や clang など）
  - [Qt バージョン 6 for Desktop](https://www.qt.io/)

### ビルド

#### WebAssembly 版
将棋丸 WebAssembly 版には将棋思考エンジンを組み込んで作成します。

1. 思考エンジンのビルド
   engines/YaneuraOu の maru ブランチで将棋思考エンジン（やねうら王）をコンパイルする。WebAssembly 向けに Makefile を適宜修正が必要。

```
 $ cd engines/YaneuraOu/source/
 $ make
```

2. 将棋丸をビルド
  Emscripten 環境を有効にした上で次のコマンドを実行する。Emscripten のバージョンは Qt をビルドしたバージョンと合わせる必要あり（ https://doc.qt.io/qt-6/wasm.html ）。

```
 $ qmake -spec wasm-emscripten CONFIG+=release
 $ make
```

#### デスクトップ版

1. qmake を実行してからビルド

```
 $ qmake CONFIG+=release
 $ make

(clang の場合)
 $ qmake -spec linux-clang CONFIG+=release
 $ make
```

Windowsの場合は build.bat も参考にしてビルドを実行してください。

将棋丸デスクトップ版を実行する際は、将棋思考エンジンを別途用意してください。

### ダウンロード

リリースされたソースコードは [リリースページ](https://github.com/shogimaru/shogimaru/releases) でダウンロードできます。

## ウェブサイト

- [将棋丸 WebAssembly 版](https://shogimaru.com)
- [将棋丸（英語版） WebAssembly 版](https://shogimaru.com/index.en.html) ... 英語版と日本語版の WebAssembly 自体は同じものです
- 将棋丸の使い方を少しずつ書いています - [操作マニュアル](https://shogimaru.github.io/shogimaru)
- Twitter [@shogimaru](https://twitter.com/shogimaru)
