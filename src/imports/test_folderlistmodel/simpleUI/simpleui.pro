#-------------------------------------------------
#
# Project created by QtCreator 2013-03-09T12:30:11
#
#-------------------------------------------------

QT       += core gui


greaterThan(QT_MAJOR_VERSION, 4) {
   QT += widgets quick
}
   
TEMPLATE = app

DEFINES += REGRESSION_TEST_FOLDERLISTMODEL

SOURCES += main.cpp  simplelist.cpp simpleslots.cpp   placesmodel.cpp terminalfolderapp.cpp \
    actionprogress.cpp

HEADERS  += simplelist.h   placesmodel.h terminalfolderapp.h \
    actionprogress.h

FORMS    += simplelist.ui

include (../../folderlistmodel/folderlistmodel.pri)

#DEFINES += DEBUG_REMOVE

DEFINES += SIMULATE_LONG_ACTION DEBUG_EXT_FS_WATCHER

RESOURCES += res.qrc



