# 将棋丸

[![ActionsCI](https://github.com/shogimaru/shogimaru/actions/workflows/actions.yml/badge.svg)](https://github.com/shogimaru/shogimaru/actions/workflows/actions.yml)
[![License](https://img.shields.io/badge/license-MIT-blue)](https://opensource.org/licenses/MIT)
[![Release](https://img.shields.io/github/v/release/shogimaru/shogimaru.svg)](https://github.com/shogimaru/shogimaru/releases)

オープンソースの将棋 GUI を C++ で作っています。

- クロスプラットフォーム - Windows, macOS, Linux, WebAssembly
- 多くの将棋思考エンジンに対応
- UI は Qt ベース
- MultiPV 対応
- 棋譜の解析モード
- 棋譜読込・保存

## 開発環境

ソースファイルをビルドして実行ファイルを作成するために、次のとおり開発環境を用意します。

- WebAssembly

  - [Emscripten](https://emscripten.org/)
  - [Qt for WebAssembly (multithread)](https://www.qt.io/)  
    Qt Company が提供している WebAssembly 向けバイナリはシングルスレッド版です（2022 年 1 月現在）。マルチスレッド版はソースからビルドする必要があります。

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

- WebAssembly  
  Emscripten(2.0.6)環境を有効にした上で次のコマンドを実行する。

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
