# Create the config.h header file
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY
)

set(SOURCES
    main.cpp
)

qt5_add_resources(RESOURCES files.qrc)

add_executable(papyros-files ${SOURCES} ${RESOURCES})
target_link_libraries(papyros-files
                      Qt5::Qml
                      Qt5::QuickControls2)

install(TARGETS papyros-files
        DESTINATION ${BIN_INSTALL_DIR})
install(FILES papyros-files.desktop
        DESTINATION ${XDG_APPS_INSTALL_DIR})
