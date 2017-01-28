QT       += testlib
TEMPLATE  = app
CONFIG   += testcase

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4) {
   QT += widgets quick
}

#------------------------------------------------

DEFINES += REGRESSION_TEST_FOLDERLISTMODEL

SOURCES += tst_folderlistmodel.cpp

include (../../folderlistmodel/folderlistmodel.pri)

HEADERS += \
    tempfiles.h

SOURCES += \
    tempfiles.cpp

INCLUDEPATH += $$PWD

# DEFINES += DEBUG_REMOVE

#DEFINES += DEBUG_EXT_FS_WATCHER

