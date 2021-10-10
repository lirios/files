liri_add_qml_plugin(FilesArchivesQmlPlugin
    MODULE_PATH
        "Liri/Files/Archives"
    OUTPUT_NAME
        archivesplugin
    SOURCES
        archives.cpp
        archives.h
        plugin.cpp
    QML_FILES
        qmldir
    DEFINES
        #QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
)

liri_finalize_qml_plugin(FilesArchivesQmlPlugin)
