QT += core gui widgets network

TARGET = glviewer
TEMPLATE = app
CONFIG += c++11


SOURCES += glviewer.cc \
    simwindow.cc \
    scene.cc

HEADERS += scene.h \
    simwindow.h
