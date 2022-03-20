# 将棋丸

[![ActionsCI](https://github.com/shogimaru/shogimaru/actions/workflows/actions.yml/badge.svg)](https://github.com/shogimaru/shogimaru/actions/workflows/actions.yml)
[![License](https://img.shields.io/badge/license-MIT-blue)](https://opensource.org/licenses/MIT)
[![Release](https://img.shields.io/github/v/release/shogimaru/shogimaru.svg)](https://github.com/shogimaru/shogimaru/releases)

ブラウザでも本格的な将棋が指せることを目指して、オープンソースの将棋 GUI を作っています。
将棋思考エンジンにはやねうら王、UI には Qt/WebAssembly を使用しています。  
棋譜の解析モードも実装されています。

## 開発環境

ソースファイルをビルドして実行ファイルを作成することができます。次のとおり開発環境を用意します。

- WebAssembly

  - [Emscripten](https://emscripten.org/)
  - [Qt for WebAssembly (multithread)](https://www.qt.io/)  
    Qt Company が提供している WebAssembly 向けバイナリはシングルスレッド版です（2022 年 1 月現在）。マルチスレッド版はソースからビルドする必要があります。

- デスクトップ版 - Windows, macOS, Linux
  - コンパイラ （gcc や clang など）
  - [Qt version 6 for Desktop](https://www.qt.io/)

### ビルド

1. クローン

```
$ git clone --recursive https://github.com/shogimaru/shogimaru.git
```

2. 将棋思考エンジン（やねうら王）をコンパイル  
   Makefile は適宜修正してください。

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

## ウェブサイト

将棋丸 - WebAssembly
https://shogimaru.com/

### Twitter

https://twitter.com/shogimaru
