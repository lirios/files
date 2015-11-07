option(USE_TAG_LIB "Use TAGLIB for getting metada" ON)

if(USE_TAG_LIB)
    pkg_check_modules(TAGLIB taglib)
    if(TAGLIB_FOUND)
        set(CMAKE_REQUIRED_INCLUDES "${TAGLIB_INCLUDE_DIRS}")
        set(CMAKE_REQUIRED_LIBRARIES "${TAGLIB_LIBRARIES}")
    else(TAGLIB_FOUND)
        add_definitions(-DDO_NOT_USE_TAG_LIB)
    endif(TAGLIB_FOUND)
else(USE_TAG_LIB)
    add_definitions(-DDO_NOT_USE_TAG_LIB)
endif(USE_TAG_LIB)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    disk
    trash
)

set(PLUGIN_DIR io/papyros/folderlistmodel)

set(folderlistmodel_SRCS
    clipboard.cpp
    clipboard.h
    diritemabstractlistmodel.h
    diriteminfo.cpp
    diriteminfo.h
    dirmodel.cpp
    dirmodel.h
    dirselection.cpp
    dirselection.h
    externalfswatcher.cpp
    externalfswatcher.h
    filecompare.cpp
    filecompare.h
    filesystemaction.cpp
    filesystemaction.h
    fmutil.cpp
    fmutil.h
    imageprovider.cpp
    imageprovider.h
    iorequest.cpp
    iorequest.h
    iorequestworker.cpp
    iorequestworker.h
    ioworkerthread.cpp
    ioworkerthread.h
    plugin.cpp
    plugin.h
    location.cpp
    location.h
    locationsfactory.cpp
    locationsfactory.h
    locationurl.cpp
    locationurl.h
    disk/disklocation.cpp
    disk/disklocation.h
    trash/qtrashdir.cpp
    trash/qtrashdir.h
    trash/qtrashutilinfo.cpp
    trash/qtrashutilinfo.h
    trash/trashiteminfo.cpp
    trash/trashiteminfo.h
    trash/trashlocation.cpp
    trash/trashlocation.h
)

add_library(folderlistmodel MODULE
    ${folderlistmodel_SRCS}
)

qt5_use_modules(folderlistmodel Gui Qml Quick Widgets)

# Copy the plugin, the qmldir file and other assets to the build dir for running in QtCreator
if(NOT "${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    add_custom_command(TARGET folderlistmodel POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/../${PLUGIN_DIR}
        COMMENT "Creating plugin directory layout in the build directory"
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/qmldir ${CMAKE_CURRENT_BINARY_DIR}/../${PLUGIN_DIR}
        COMMENT "Copying the qmldir file to the build directory"
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:folderlistmodel> ${CMAKE_CURRENT_BINARY_DIR}/../${PLUGIN_DIR}
        COMMENT "Copying the plugin binary to the build directory"
    )
endif(NOT "${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")

# Install plugin file
install(TARGETS folderlistmodel DESTINATION ${QT_IMPORTS_DIR}/io/papyros/folderlistmodel/)
install(FILES qmldir DESTINATION ${QT_IMPORTS_DIR}/io/papyros/folderlistmodel/)

