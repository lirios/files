TARGET = papyros-files

QT += quick qml
!no_desktop: QT += widgets

SOURCES += src/main.cpp

OTHER_FILES = COPYING \
    README.md

RESOURCES += files-app.qrc

target.path = /usr/bin

desktop.files = papyros-files.desktop
desktop.path = /usr/share/applications

INSTALLS += target desktop
