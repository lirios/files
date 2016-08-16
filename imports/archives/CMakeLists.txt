include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(PLUGIN_DIR com/ubuntu/Archives)

set(archives_SRCS
    archives.cpp
    archives.h
    archives_plugin.cpp
    archives_plugin.h
)

add_library(Archives MODULE
    ${archives_SRCS}
)

qt5_use_modules(Archives Gui Qml Quick Widgets)

# Copy the plugin, the qmldir file and other assets to the build dir for running in QtCreator
if(NOT "${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    add_custom_command(TARGET Archives POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/../${PLUGIN_DIR}
        COMMENT "Creating plugin directory layout in the build directory"
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/qmldir ${CMAKE_CURRENT_BINARY_DIR}/../${PLUGIN_DIR}
        COMMENT "Copying the qmldir file to the build directory"
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Archives> ${CMAKE_CURRENT_BINARY_DIR}/../${PLUGIN_DIR}
        COMMENT "Copying the plugin binary to the build directory"
    )
endif(NOT "${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")

# Install plugin file
install(TARGETS Archives DESTINATION ${QT_IMPORTS_DIR}/${PLUGIN_DIR})
install(FILES qmldir DESTINATION ${QT_IMPORTS_DIR}/${PLUGIN_DIR})

