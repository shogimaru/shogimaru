TARGET   = shogimaru
TEMPLATE = app
QT      += core gui widgets network
CONFIG  += c++17
CODECFORTR = UTF-8
MOC_DIR  = .obj/
OBJECTS_DIR = .obj/
DEFINES += QT_DEPRECATED_WARNINGS
INCLUDEPATH += src
UI_DIR = src/

!CONFIG(debug, debug|release) {
  DEFINES += QT_NO_DEBUG_OUTPUT
}

wasm {
  CONFIG +=
  LIBS = engines/YaneuraOu/source/YaneuraOu.wasm
  SOURCES += src/maincontroller_wasm.cpp
  SOURCES += src/file_wasm.cpp
  SOURCES += src/engine_wasm.cpp
  SOURCES += src/command_wasm.cpp
  SOURCES += src/commandlineparser_wasm.cpp
  SOURCES += src/sound_sdl.cpp
  HEADERS += src/stringconverter.h
  SOURCES += src/stringconverter_iconv.cpp
  HEADERS += src/enginethread.h
  SOURCES += src/enginethread.cpp
  DESTDIR  = html/
} else {
  QT      += multimedia
  SOURCES += src/maincontroller_native.cpp
  SOURCES += src/file.cpp
  SOURCES += src/engine_native.cpp
  SOURCES += src/command_native.cpp
  SOURCES += src/commandlineparser_native.cpp
  SOURCES += src/sound_native.cpp
  HEADERS += src/engineprocess.h
  SOURCES += src/engineprocess.cpp

  HEADERS += src/stringconverter.h
  msvc {
    SOURCES += src/stringconverter_win.cpp
    DESTDIR = dist/
  } else {
    SOURCES += src/stringconverter_iconv.cpp
    DESTDIR  = ./
  }
}

msvc {
  QMAKE_CXXFLAGS += /std:c++17 /Zc:__cplusplus /utf-8
  RC_FILE = resources/windows/app.rc
}

macx {
  LIBS += -liconv
  RESOURCE_DIR = $${TARGET}.app/Contents/Resources
  QMAKE_POST_LINK += mkdir -p $${RESOURCE_DIR}/assets/images/ && cp -a $$PWD/assets/images/*  $${RESOURCE_DIR}/assets/images/;
  QMAKE_POST_LINK += mkdir -p $${RESOURCE_DIR}/assets/sounds/ && cp -a $$PWD/assets/sounds/*  $${RESOURCE_DIR}/assets/sounds/;
  QMAKE_POST_LINK += mkdir -p $${RESOURCE_DIR}/assets/translations/ && cp -a $$PWD/assets/translations/*  $${RESOURCE_DIR}/assets/translations/;

  QMAKE_INFO_PLIST = resources/macos/Info.plist
  ICON = resources/macos/shogimaru.icns
}

SOURCES += \
        src/main.cpp \
        src/global.cpp \
        src/command.cpp \
        src/usibus.cpp \
        src/engine.cpp \
        src/piece.cpp \
        src/board.cpp \
        src/chessclock.cpp \
        src/ponderinfo.cpp \
        src/shogirecord.cpp \
        src/badge.cpp \
        src/sfen.cpp \
        src/player.cpp \
        src/user.cpp \
        src/recorder.cpp \
        src/kifu.cpp \
        src/promotiondialog.cpp \
        src/nicknamedialog.cpp \
        src/analysisdialog.cpp \
        src/recorddialog.cpp \
        src/settingsdialog.cpp \
        src/enginesettings.cpp \
        src/westerntabstyle.cpp \
        src/startdialog.cpp \
        src/startdialog2.cpp \
        src/mypage.cpp \
        src/messagebox.cpp \
        src/evaluationgraph.cpp \
        src/operationbuttongroup.cpp \
        src/processingdialog.cpp \
        src/environmentvariablesdialog.cpp \
        src/userenvironmentvariabledialog.cpp \
        src/browseenginedialog.cpp \
        src/maincontroller.cpp

HEADERS += \
        src/global.h \
        src/command.h \
        src/usibus.h \
        src/engine.h \
        src/piece.h \
        src/board.h \
        src/chessclock.h \
        src/ponderinfo.h \
        src/shogirecord.h \
        src/badge.h \
        src/sfen.h \
        src/player.h \
        src/user.h \
        src/recorder.h \
        src/kifu.h\
        src/promotiondialog.h \
        src/nicknamedialog.h \
        src/analysisdialog.h \
        src/recorddialog.h \
        src/settingsdialog.h \
        src/enginesettings.h \
        src/westerntabstyle.h \
        src/startdialog.h \
        src/startdialog2.h \
        src/mypage.h \
        src/messagebox.h \
        src/file.h \
        src/sound.h \
        src/evaluationgraph.h \
        src/operationbuttongroup.h \
        src/commandlineparser.h \
        src/processingdialog.h \
        src/environmentvariablesdialog.h \
        src/userenvironmentvariabledialog.h \
        src/browseenginedialog.h \
        src/maincontroller.h

FORMS += \
        forms/mainwindow.ui \
        forms/mypage.ui \
        forms/nicknamedialog.ui \
        forms/analysisdialog.ui \
        forms/recorddialog.ui \
        forms/settingsdialog.ui \
        forms/operationbuttongroup.ui \
        forms/environmentvariablesdialog.ui \
        forms/userenvironmentvariabledialog.ui \
        forms/browseenginedialog.ui \
        forms/startdialog.ui \
        forms/startdialog2.ui

TRANSLATIONS += \
        src/message_ja.ts

# Default rules for deployment.
!isEmpty(target.path): INSTALLS += target
