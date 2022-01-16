TARGET   = shogimaru
TEMPLATE = app
QT      += core gui widgets
CONFIG  += c++17
CODECFORTR = UTF-8
MOC_DIR  = .obj/
OBJECTS_DIR = .obj/
DEFINES += QT_DEPRECATED_WARNINGS

wasm {
  CONFIG +=
  LIBS = ../engines/YaneuraOu/source/YaneuraOu.wasm
  SOURCES += file_wasm.cpp
  DESTDIR  = ../html/
} else {
  LIBS = ../engines/YaneuraOu/source/YaneuraOu.a -lSDL -lSDL_mixer
  SOURCES += file.cpp
  DESTDIR  = ../
}

SOURCES += \
        main.cpp \
        global.cpp \
        command.cpp \
        usibus.cpp \
        engine.cpp \
        enginethread.cpp \
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
#        startdialog.cpp \
        startdialog2.cpp \
        mypage.cpp \
        messagebox.cpp \
        sound.cpp \
        evaluationgraph.cpp \
        maincontroller.cpp

HEADERS += \
        global.h \
        command.h \
        usibus.h \
        engine.h \
        enginethread.h \
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
#        startdialog.h \
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
        ../forms/startdialog.ui \
        ../forms/startdialog2.ui

TRANSLATIONS += \
        message_ja.ts

# Default rules for deployment.
!isEmpty(target.path): INSTALLS += target
