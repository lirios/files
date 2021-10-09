# Translations
file(GLOB LiriFiles_TRANSLATIONS "${CMAKE_SOURCE_DIR}/translations/*_*.ts")
qt5_add_translation(LiriFiles_QM_FILES ${LiriFiles_TRANSLATIONS})
install(FILES ${LiriFiles_QM_FILES}
        DESTINATION "${INSTALL_DATADIR}/liri-files/translations")

liri_add_executable(LiriFiles
    OUTPUT_NAME
        "liri-files"
    SOURCES
        main.cpp
        ${LiriFiles_QM_FILES}
    RESOURCES
        files.qrc
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
        FILES_VERSION="${PROJECT_VERSION}"
    APPDATA
        "${CMAKE_CURRENT_SOURCE_DIR}/io.liri.Files.appdata.xml"
    DESKTOP
        "${CMAKE_CURRENT_SOURCE_DIR}/io.liri.Files.desktop"
    LIBRARIES
        Qt5::Core
        Qt5::Gui
        Qt5::Qml
        Qt5::Quick
        Qt5::QuickControls2
    GUI
)

liri_finalize_executable(LiriFiles)
