TARGET = marutest
TEMPLATE = app
CONFIG += console c++17 testcase
CONFIG -= app_bundle
QT += testlib widgets
QT -= gui

SOURCES  = main.cpp
SOURCES += ../global.cpp
SOURCES += ../sfen.cpp
SOURCES += ../shogirecord.cpp
