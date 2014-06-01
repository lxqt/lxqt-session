project( lxqt-session )
cmake_minimum_required( VERSION 2.6 )

if (NOT CMAKE_BUILD_TYPE)
   set ( CMAKE_BUILD_TYPE Release )
endif (NOT CMAKE_BUILD_TYPE)

add_definitions ( -Wall ) # FIXME: this is compiler specific

if(USE_QT4)
	set(QTX_INCLUDE_DIRS ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${QT_QTDBUS_INCLUDE_DIR} ${QT_QTXML_INCLUDE_DIR})
	set(QTX_LIBRARIES ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTDBUS_LIBRARY})
elseif(USE_QT5)
	set(QTX_INCLUDE_DIRS ${Qt5Widgets_INCLUDE_DIRS} ${Qt5DBus_INCLUDE_DIRS} ${Qt5X11Extras_INCLUDE_DIRS})
	set(QTX_LIBRARIES ${Qt5Widgets_LIBRARIES} ${Qt5DBus_LIBRARIES} ${Qt5X11Extras_LIBRARIES})
endif()

include(LxQtLibSuffix)

include_directories (
    ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
    ${QTX_INCLUDE_DIRS} ${XCB_INCLUDE_DIRS} ${X11_INCLUDE_DIR}
    src)

set ( lxqt-session_HDRS
)

set ( lxqt-session_SRCS
    src/main.cpp
    src/lxqtmodman.cpp
    src/wmselectdialog.cpp
    src/windowmanager.cpp
    src/sessionapplication.cpp
)

set ( lxqt-session_MOCS
    src/lxqtmodman.h
    src/sessiondbusadaptor.h
    src/wmselectdialog.h
    src/sessionapplication.h
)

set ( lxqt-session_UI
    src/wmselectdialog.ui
)

# Translations **********************************
include(LxQtTranslate)
lxqt_translate_ts(lxqt-runner_QM_FILES 
    SOURCES
        ${lxqt-session_HDRS}
        ${lxqt-session_SRCS} 
        ${lxqt-session_UI}
)

#************************************************

if(USE_QT4)
	qt4_wrap_cpp(MOCS ${lxqt-session_MOCS})
	qt4_wrap_ui(UIS ${lxqt-session_UI})
elseif(USE_QT5)
	qt5_wrap_cpp(MOCS ${lxqt-session_MOCS})
	qt5_wrap_ui(UIS ${lxqt-session_UI})
endif()

add_executable ( lxqt-session ${lxqt-session_SRCS} ${UIS} ${RSCS} ${lxqt-runner_QM_FILES} ${MOCS} )
add_dependencies( lxqt-session lxqt qtxdg)
target_link_libraries ( lxqt-session ${XCB_LIBRARIES} ${X11_LIBRARIES} ${QTX_LIBRARIES} ${LXQT_LIBRARIES} ${QTXDG_LIBRARIES})
INSTALL(TARGETS lxqt-session RUNTIME DESTINATION bin)

if (NOT DEFINED BUNDLE_XDG_UTILS)
    message(STATUS "*********************************************************************")
    message(STATUS "Bundling our own xdg-utils.")
    message(STATUS "If you have xdg-utils newer than 2012-03-02, you can set -DBUNDLE_XDG_UTILS=No")
    message(STATUS "*********************************************************************")
    set (BUNDLE_XDG_UTILS Yes)
endif (NOT DEFINED BUNDLE_XDG_UTILS)

if (BUNDLE_XDG_UTILS)
    install(PROGRAMS resources/xdg-open resources/xdg-mime DESTINATION lib${LIB_SUFFIX}/lxqt-xdg-tools)
    add_definitions ( -DPATH_PREPEND=\"${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/lxqt-xdg-tools\" )
endif (BUNDLE_XDG_UTILS)

# install(PROGRAMS src/startlxqt DESTINATION bin)

