project( lxqt-session )
cmake_minimum_required( VERSION 2.6 )

if (NOT CMAKE_BUILD_TYPE)
   set ( CMAKE_BUILD_TYPE Release )
endif (NOT CMAKE_BUILD_TYPE)

add_definitions ( -Wall )
find_package ( Qt4 REQUIRED )
find_package ( X11 REQUIRED )

include(LxQtLibSuffix)

include ( ${QT_USE_FILE} )
include_directories (
    ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
    ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${QT_QTDBUS_INCLUDE_DIR}
    ${X11_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/libraries/
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

QT4_WRAP_CPP(MOCS ${lxqt-session_MOCS})
qt4_wrap_ui(UIS ${lxqt-session_UI})

add_executable ( lxqt-session ${lxqt-session_SRCS} ${UIS} ${RSCS} ${lxqt-runner_QM_FILES} ${MOCS} )
add_dependencies( lxqt-session lxqt qtxdg)
target_link_libraries ( lxqt-session  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTDBUS_LIBRARY} lxqt qtxdg)
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

