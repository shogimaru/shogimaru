TARGET   = shogimaru
TEMPLATE = app
QT      += core gui widgets
CONFIG  += c++17
CODECFORTR = UTF-8
MOC_DIR  = .obj/
OBJECTS_DIR = .obj/

wasm {
  CONFIG +=
#  QT.global.enabled_features += thread
  LIBS = engines/YaneuraOu/YaneuraOu.wasm
  QMAKE_CXXFLAGS +=
  QMAKE_LFLAGS += --preload-file assets/
  QMAKE_LFLAGS += -lidbfs.js
  QMAKE_LFLAGS += -s ASYNCIFY=1
  QMAKE_WASM_PTHREAD_POOL_SIZE=16
  QMAKE_WASM_TOTAL_MEMORY=900MB
#  QMAKE_LFLAGS += -s TOTAL_MEMORY=1GB
  # これはいずれ消したい
#  QMAKE_LFLAGS += -Wl,--shared-memory,--no-check-features
#  QMAKE_LFLAGS += -Wl,--no-check-features
  SOURCES += file_wasm.cpp
  DESTDIR = ../html/
} else {
  LIBS     = ../engines/YaneuraOu/YaneuraOu.a -lSDL -lSDL_mixer
  SOURCES += file.cpp
  DESTDIR = ../
}

DEFINES += QT_DEPRECATED_WARNINGS

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
