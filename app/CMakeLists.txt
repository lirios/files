file(GLOB_RECURSE QML_SOURCES *.qml *.js)

set(SOURCES
    main.cpp
)

add_executable(papyros-files ${SOURCES} ${QML_SOURCES})
target_link_libraries(papyros-files
                      Qt5::Qml
                      Qt5::Quick)

install(TARGETS papyros-files
        DESTINATION ${BIN_INSTALL_DIR})
install(FILES papyros-files.desktop
        DESTINATION ${XDG_APPS_INSTALL_DIR})
