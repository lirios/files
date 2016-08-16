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
    disk
    trash
)

set(SOURCES
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

add_library(folderlistmodelplugin SHARED ${SOURCES})
target_link_libraries(folderlistmodelplugin
                      Qt5::Widgets
                      Qt5::Qml
                      Qt5::Quick)

if(USE_TAG_LIB)
    target_link_libraries(folderlistmodelplugin ${TAGLIB_LIBRARIES})
endif()

install(TARGETS folderlistmodelplugin
        DESTINATION ${QML_INSTALL_DIR}/Papyros/Files/FolderListModel)
install(FILES qmldir
        DESTINATION ${QML_INSTALL_DIR}/Papyros/Files/FolderListModel)
