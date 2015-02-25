include(FindGettext)
find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)

set(EXTOPTS )

set(DOMAIN ${PROJECT_NAME})
set(POT_FILE ${DOMAIN}.pot)
file(GLOB PO_FILES *.po)

add_custom_target(${POT_FILE} ALL
    COMMENT "Generating translation template..."
    COMMAND ${INTLTOOL_EXTRACT} --update --type=gettext/ini
        --srcdir=${CMAKE_SOURCE_DIR} ${DESKTOP_FILE}.in.in
    COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE} -o ${POT_FILE}
        -D ${CMAKE_CURRENT_SOURCE_DIR}
        -D ${CMAKE_CURRENT_BINARY_DIR}
        --c++ --kde
        --from-code=UTF-8
        -ci18n
        -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3
        -kki18n:1 -kki18nc:1c,2 -kki18np:1,2 -kki18ncp:1c,2,3
        -ktr -ktr:1,2 -kN_
        -kI18N_NOOP:1 -kI18NC_NOOP:1c,2
        --package-name='${PROJECT}'
        --copyright-holder=Papyros
        --msgid-bugs-address=support@papyros.io
        ${I18N_SRC_FILES}
    COMMAND ${CMAKE_COMMAND} -E copy ${POT_FILE} ${CMAKE_CURRENT_SOURCE_DIR})

foreach(PO_FILE ${PO_FILES})
    get_filename_component(LANG ${PO_FILE} NAME_WE)
    gettext_process_po_files(${LANG} ALL PO_FILES ${PO_FILE})
    set(INSTALL_DIR ${CMAKE_INSTALL_LOCALEDIR}/${LANG}/LC_MESSAGES)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${LANG}.gmo
            DESTINATION ${INSTALL_DIR}
            RENAME ${DOMAIN}.mo)
endforeach(PO_FILE)
