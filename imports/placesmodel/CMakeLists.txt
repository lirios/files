set(SOURCES
    placesmodel.cpp
    placesmodel.h
    placesmodel_plugin.cpp
    placesmodel_plugin.h
    qmtabparser.h
    qmtabparser.cpp
)

add_library(placesmodelplugin SHARED ${SOURCES})
target_link_libraries(placesmodelplugin
                      Qt5::Widgets
                      Qt5::Qml
                      Qt5::Quick)

install(TARGETS placesmodelplugin
        DESTINATION ${QML_INSTALL_DIR}/Liri/Files/PlacesModel)
install(FILES qmldir
        DESTINATION ${QML_INSTALL_DIR}/Liri/Files/PlacesModel)
