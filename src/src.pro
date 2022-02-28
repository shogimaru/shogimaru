TARGET   = shogimaru
TEMPLATE = app
QT      += core gui widgets network
CONFIG  += c++17
CODECFORTR = UTF-8
MOC_DIR  = .obj/
OBJECTS_DIR = .obj/
DEFINES += QT_DEPRECATED_WARNINGS

wasm {
  CONFIG +=
  LIBS = ../engines/YaneuraOu/source/YaneuraOu.wasm
  SOURCES += maincontroller_wasm.cpp
  SOURCES += file_wasm.cpp
  SOURCES += engine_wasm.cpp
  SOURCES += command_wasm.cpp
  SOURCES += sound_sdl.cpp
  HEADERS += enginethread.h
  SOURCES += enginethread.cpp
  DESTDIR  = ../html/
} else {
  QT      += core5compat multimedia
  SOURCES += maincontroller_native.cpp
  SOURCES += file.cpp
  SOURCES += engine_native.cpp
  SOURCES += command_native.cpp
  SOURCES += sound_native.cpp
  HEADERS += engineprocess.h
  SOURCES += engineprocess.cpp
  DESTDIR  = ../
}

msvc {
  QMAKE_CXXFLAGS += /std:c++17 /Zc:__cplusplus
}

SOURCES += \
        main.cpp \
        global.cpp \
        command.cpp \
        usibus.cpp \
        engine.cpp \
        piece.cpp \
        board.cpp \
        chessclock.cpp \
        ponderinfo.cpp \
        shogirecord.cpp \
        badge.cpp \
        sfen.cpp \
        player.cpp \
        user.cpp \
        recorder.cpp \
        kifu.cpp \
        promotiondialog.cpp \
        nicknamedialog.cpp \
        analysisdialog.cpp \
        recorddialog.cpp \
        settingsdialog.cpp \
        enginesettings.cpp \
        westerntabstyle.cpp \
        startdialog2.cpp \
        mypage.cpp \
        messagebox.cpp \
        evaluationgraph.cpp \
        maincontroller.cpp

HEADERS += \
        global.h \
        command.h \
        usibus.h \
        engine.h \
        piece.h \
        board.h \
        chessclock.h \
        ponderinfo.h \
        shogirecord.h \
        badge.h \
        sfen.h \
        player.h \
        user.h \
        recorder.h \
        kifu.h\
        promotiondialog.h \
        nicknamedialog.h \
        analysisdialog.h \
        recorddialog.h \
        settingsdialog.h \
        enginesettings.h \
        westerntabstyle.h \
        startdialog2.h \
        mypage.h \
        messagebox.h \
        file.h \
        sound.h \
        evaluationgraph.h \
        maincontroller.h

FORMS += \
        ../forms/mainwindow.ui \
        ../forms/mypage.ui \
        ../forms/nicknamedialog.ui \
        ../forms/analysisdialog.ui \
        ../forms/recorddialog.ui \
        ../forms/settingsdialog.ui \
#        ../forms/startdialog.ui \
        ../forms/startdialog2.ui

TRANSLATIONS += \
        message_ja.ts

# Default rules for deployment.
!isEmpty(target.path): INSTALLS += target
