find_package (Qt5LinguistTools)

file(GLOB TRANSLATIONS pt_PT.ts)
file(GLOB_RECURSE TO_BE_TRANSLATED ${CMAKE_SOURCE_DIR}/src/*.qml
                                   ${CMAKE_SOURCE_DIR}/src/*.cpp
                                   ${CMAKE_SOURCE_DIR}/src/*.h)

option(UPDATE_TRANSLATIONS "Update .ts files with new strings to translate." OFF)

if(UPDATE_TRANSLATIONS)
    qt5_create_translation(QM_FILES ${TO_BE_TRANSLATED} ${TRANSLATIONS})
else(UPDATE_TRANSLATIONS)
    qt5_add_translation(QM_FILES ${TRANSLATIONS})
endif(UPDATE_TRANSLATIONS)

add_custom_target(translations ALL DEPENDS ${QM_FILES})

install(FILES ${QM_FILES} DESTINATION ${DATA_DIR}/translations)
