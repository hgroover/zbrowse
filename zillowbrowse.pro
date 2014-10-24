#-------------------------------------------------
#
# Project created by QtCreator 2014-07-31T19:02:20
#
#-------------------------------------------------

QT       += core gui
QT       += network
QT       += webkit
QT       += webkitwidgets
QT       += sql
QT       += printsupport
QT       += multimedia

include( ../QtXlsxWriter/src/xlsx/qtxlsx.pri )

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = zillowbrowse
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    dlglinkchooser.cpp \
    sqlfns.cpp \
    parsefns.cpp \
    saverestore.cpp \
    audio.cpp \
    dlgusercolumns.cpp \
    dlgweight.cpp

HEADERS  += mainwindow.h \
    dlglinkchooser.h \
    sqldefs.h \
    columnids.h \
    versioninfo.h \
    dlgusercolumns.h \
    dlgweight.h

FORMS    += mainwindow.ui \
    dlglinkchooser.ui \
    dlgusercolumns.ui \
    dlgweight.ui

OTHER_FILES += \
    README.md \
    changelog.txt
