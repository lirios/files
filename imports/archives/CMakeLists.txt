set(SOURCES
    archives.cpp
    plugin.cpp
)

add_library(archivesplugin SHARED ${SOURCES})
target_link_libraries(archivesplugin
                      Qt5::Widgets
                      Qt5::Qml
                      Qt5::Quick)

install(TARGETS archivesplugin
        DESTINATION ${QML_INSTALL_DIR}/Papyros/Files/Archives)
install(FILES qmldir
        DESTINATION ${QML_INSTALL_DIR}/Papyros/Files/Archives)
