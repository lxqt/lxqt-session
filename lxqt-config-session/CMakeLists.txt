project(lxqt-config-session)

add_definitions ( -Wall )
find_package ( Qt4 REQUIRED )
include ( ${QT_USE_FILE} )

include_directories (
    ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
    ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR}
    ${QT_QTDBUS_INCLUDE_DIR} ${QT_QTXML_INCLUDE_DIR}
    ${X11_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/libraries/)

set( lxqt-sessioncfg_HDRS
    sessionconfigwindow.h
    basicsettings.h
    modulemodel.h
    autostartpage.h
    autostartmodel.h
    autostartitem.h
    autostartedit.h
    defaultappspage.h
    environmentpage.h
)

set( lxqt-sessioncfg_MOCS
    sessionconfigwindow.h
    basicsettings.h
    modulemodel.h
    autostartpage.h
    autostartmodel.h
    autostartedit.h
    defaultappspage.h
    environmentpage.h
)

set( lxqt-sessioncfg_SRCS
    main.cpp
    sessionconfigwindow.cpp
    basicsettings.cpp
    modulemodel.cpp
    autostartpage.cpp
    autostartmodel.cpp
    autostartitem.cpp
    autostartedit.cpp
    defaultappspage.cpp
    environmentpage.cpp
    ../lxqt-session/src/windowmanager.cpp
)

set(lxqt-sessioncfg_UI_FILES
    basicsettings.ui
    autostartpage.ui
    autostartedit.ui
    defaultappspage.ui
    environmentpage.ui
)

QT4_WRAP_UI(lxqt-sessioncfg_UIS ${lxqt-sessioncfg_UI_FILES})
QT4_WRAP_CPP(lxqt-sessioncfg_MOC_SOURCES ${lxqt-sessioncfg_MOCS})
# Translations **********************************
include(LxQtTranslate)
lxqt_translate_ts(lxqt-sessioncfg_QM_FILES 
    SOURCES
        ${lxqt-sessioncfg_HDRS}
        ${lxqt-sessioncfg_SRCS} 
        ${lxqt-sessioncfg_UI_FILES}
)

lxqt_translate_desktop(lxqt-sessioncfg_DESKTOP_FILES
    SOURCES
        lxqt-config-session.desktop.in
) 
#************************************************

add_executable ( lxqt-config-session ${lxqt-sessioncfg_SRCS} ${lxqt-sessioncfg_UIS} ${lxqt-sessioncfg_MOC_SOURCES} ${lxqt-sessioncfg_DESKTOP_FILES} ${lxqt-sessioncfg_QM_FILES})
add_dependencies( lxqt-config-session lxqt qtxdg)
target_link_libraries( lxqt-config-session  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTDBUS_LIBRARY} lxqt qtxdg)

INSTALL(TARGETS lxqt-config-session RUNTIME DESTINATION bin)
install(FILES ${lxqt-sessioncfg_DESKTOP_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
