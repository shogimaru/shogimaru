# 将棋丸

[![ActionsCI](https://github.com/shogimaru/shogimaru/actions/workflows/actions.yml/badge.svg)](https://github.com/shogimaru/shogimaru/actions/workflows/actions.yml)
[![License](https://img.shields.io/badge/license-MIT-blue)](https://opensource.org/licenses/MIT)
[![Release](https://img.shields.io/github/v/release/shogimaru/shogimaru.svg)](https://github.com/shogimaru/shogimaru/releases)

オープンソースの将棋 GUI を C++ で作っています。
思考エンジンと通信するための USI プロトコルに対応しており、次の特徴があります。

- マルチプラットフォーム - Windows, macOS, Linux, WebAssembly
- MultiPV 表示
- UI は Qt ベース
- 棋譜の解析モード
- 棋譜読込・保存
- 多言語対応（日本語、英語）

## 開発環境

ソースファイルをビルドして実行ファイルを作成するために、次のとおり開発環境を用意します。

- WebAssembly

  - [Emscripten](https://emscripten.org/)
  - [Qt for WebAssembly (multithread)](https://www.qt.io/)  

- デスクトップ版 - Windows, macOS, Linux
  - コンパイラ （gcc や clang など）
  - [Qt バージョン 6 for Desktop](https://www.qt.io/)

### ビルド

#### WebAssembly 版

1. リポジトリ取得

```
 $ git clone --recursive https://github.com/shogimaru/shogimaru.git
```

2. 将棋思考エンジン（やねうら王）をコンパイル  
   WebAssembly 向けに Makefile を適宜修正する。

```
 $ cd engines/YaneuraOu/source/
 $ make
```

3. 将棋丸をコンパイル

- WebAssembly（マルチスレッド版）
  Emscripten 環境を有効にした上で次のコマンドを実行する。
  Emscripten のバージョンは Qt をビルドしたバージョンと合わせること（https://doc.qt.io/qt-6/wasm.html）。

```
 $ qmake -spec wasm-emscripten CONFIG+=release && make
```

#### デスクトップ版

1. リポジトリ取得

```
 $ git clone --recursive https://github.com/shogimaru/shogimaru.git
```

2. 将棋丸をコンパイル

```
 $ qmake CONFIG+=release
 $ make

(clang の場合)
 $ qmake -spec linux-clang CONFIG+=release
 $ make
```

※ 将棋思考エンジンには公開されている実行ファイルを使えばよいので、ここではコンパイルする必要はない

### ダウンロード

リリースされたソースコードやバイナリは [こちら](https://github.com/shogimaru/shogimaru/releases) でダウンロードできます。

## ウェブサイト

- [将棋丸 WebAssembly 版](https://shogimaru.com)
- [将棋丸（英語版） WebAssembly 版](https://shogimaru.com/index.en.html) ... 英語版と日本語版の WebAssembly 自体は同じものです
- 将棋丸の使い方を少しずつ書いています - [操作マニュアル](https://shogimaru.github.io/shogimaru)
- Twitter [@shogimaru](https://twitter.com/shogimaru)
