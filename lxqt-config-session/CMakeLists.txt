project(lxqt-config-session)

add_definitions ( -Wall ) # FIXME: this is compiler specific

if(USE_QT4)
	set(QTX_INCLUDE_DIRS ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${QT_QTDBUS_INCLUDE_DIR} ${QT_QTXML_INCLUDE_DIR})
	set(QTX_LIBRARIES ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTDBUS_LIBRARY})
elseif(USE_QT5)
	set(QTX_INCLUDE_DIRS ${Qt5Widgets_INCLUDE_DIRS} ${Qt5DBus_INCLUDE_DIRS} ${Qt5X11Extras_INCLUDE_DIRS})
	set(QTX_LIBRARIES ${Qt5Widgets_LIBRARIES} ${Qt5DBus_LIBRARIES} ${Qt5X11Extras_LIBRARIES})
endif()

include_directories (
    ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
    ${QTX_INCLUDE_DIRS})

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

if(USE_QT4)
	qt4_wrap_ui(lxqt-sessioncfg_UIS ${lxqt-sessioncfg_UI_FILES})
	qt4_wrap_cpp(lxqt-sessioncfg_MOC_SOURCES ${lxqt-sessioncfg_MOCS})
elseif(USE_QT5)
	qt5_wrap_ui(lxqt-sessioncfg_UIS ${lxqt-sessioncfg_UI_FILES})
	qt5_wrap_cpp(lxqt-sessioncfg_MOC_SOURCES ${lxqt-sessioncfg_MOCS})
endif()

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
target_link_libraries( lxqt-config-session ${QTX_LIBRARIES} ${LXQT_LIBRARIES} ${QTXDG_LIBRARIES})

INSTALL(TARGETS lxqt-config-session RUNTIME DESTINATION bin)
install(FILES ${lxqt-sessioncfg_DESKTOP_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
