TARGET = marutest
TEMPLATE = app
CONFIG += console c++14 testcase
CONFIG -= app_bundle
QT += testlib
QT -= gui

SOURCES  = main.cpp
SOURCES += ../sfen.cpp
SOURCES += ../shogirecord.cpp
