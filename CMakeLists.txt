project(io.papyros.files C CXX)

cmake_minimum_required(VERSION 2.8.9)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-permissive -pedantic -Wall -Wextra -fPIC")

# Standard install paths
include(GNUInstallDirs)

option(INSTALL_TESTS "Install the tests on make install" on)

set(APP_NAME papyros-files)
set(DESKTOP_FILE "${PROJECT_NAME}.desktop")
# set(LP_PROJECT ubuntu-filemanager-app)
# set(ICON_FILE system-file-manager)
# set(AUTOPILOT_DIR ${APP_NAME})
set(EXEC "${APP_NAME}")

execute_process(
    COMMAND qmake -query QT_INSTALL_QML
    OUTPUT_VARIABLE QT_IMPORTS_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(DATA_DIR ${CMAKE_INSTALL_DATADIR}/${APP_NAME})
# set(ICON ${CMAKE_INSTALL_PREFIX}/${DATA_DIR}/${ICON_FILE})
set(ICON system-file-manager)
set(DESKTOP_DIR ${CMAKE_INSTALL_DATADIR}/applications)

configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/config.h)
include_directories( ${CMAKE_BINARY_DIR} )

file(GLOB SRC_FILES
     RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
     *.qml *.js *.png *.svg *.json)
install(FILES ${SRC_FILES}  DESTINATION ${DATA_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE}
		DESTINATION ${DESKTOP_DIR})

configure_file(${DESKTOP_FILE}.in ${DESKTOP_FILE})

# Tests
enable_testing()

add_subdirectory(ts)
add_subdirectory(tests)
add_subdirectory(src)
