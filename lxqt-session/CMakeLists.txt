project( lxqt-session )

if (NOT CMAKE_BUILD_TYPE)
   set ( CMAKE_BUILD_TYPE Release )
endif (NOT CMAKE_BUILD_TYPE)

add_definitions ( -Wall ) # FIXME: this is compiler specific

set(QTX_INCLUDE_DIRS "")
set(QTX_LIBRARIES Qt5::Widgets Qt5::DBus Qt5::X11Extras)

include(LxQtLibSuffix)

add_definitions(-DPROJECT_NAME=\"${PROJECT_NAME}\")

include_directories (
    ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR} ${QTX_INCLUDE_DIRS}
    src)

set ( lxqt-session_HDRS
)

set ( lxqt-session_SRCS
    src/main.cpp
    src/lxqtmodman.cpp
    src/wmselectdialog.cpp
    src/windowmanager.cpp
    src/sessionapplication.cpp
    src/sessiondbusadaptor.h
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
    INSTALLATION_DIR
        ${LXQT_TRANSLATIONS_DIR}/${PROJECT_NAME}
)

#************************************************

qt5_wrap_ui(UIS ${lxqt-session_UI})

add_executable ( lxqt-session ${lxqt-session_SRCS} ${UIS} ${RSCS} ${lxqt-runner_QM_FILES} )
target_link_libraries ( lxqt-session ${QTX_LIBRARIES} ${LXQT_LIBRARIES} ${QTXDG_LIBRARIES} KF5::WindowSystem)
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
