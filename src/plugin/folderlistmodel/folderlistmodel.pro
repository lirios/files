TARGET = nemofolderlistmodel

PLUGIN_IMPORT_PATH = org/nemomobile/folderlistmodel

#core:  sources + headers, separated here to use in regression test project
include (folderlistmodel.pri)

# Input
SOURCES += plugin.cpp
HEADERS += plugin.h

## QApplication::clipboard() needs gui
QT += gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

exists(../plugin.pri) {
   include(../plugin.pri)
}
else {
    TEMPLATE = lib
    CONFIG += qt plugin hide_symbols
    greaterThan(QT_MAJOR_VERSION, 4) {
       QT += qml quick
    }
    else {
       QT += declarative
    }
    target.path = $$[QT_INSTALL_QML]/$$PLUGIN_IMPORT_PATH
    INSTALLS += target
    qmldir.files += $$PWD/qmldir
    qmldir.path +=  $$[QT_INSTALL_QML]/$$$$PLUGIN_IMPORT_PATH
    INSTALLS += qmldir
}


