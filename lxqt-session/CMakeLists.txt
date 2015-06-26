cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)
project(lxqt-session)

if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release)
endif()

if(NOT DEFINED LIBUDEV_MONITOR OR LIBUDEV_MONITOR)
    set(LIBUDEV_MONITOR Yes)

    find_package(PkgConfig)
    pkg_check_modules(LIBUDEV REQUIRED libudev)

    add_definitions(-DWITH_LIBUDEV_MONITOR)
endif()

include(LxQtLibSuffix)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${XCB_INCLUDE_DIRS}
    ${X11_INCLUDE_DIR}
    src
)
if(LIBUDEV_MONITOR)
    include_directories(${LIBUDEV_INCLUDE_DIRS})
endif()

set(lxqt-session_HDRS "")

set(lxqt-session_SRCS
    src/main.cpp
    src/lxqtmodman.cpp
    src/wmselectdialog.cpp
    src/windowmanager.cpp
    src/sessionapplication.cpp
    src/sessiondbusadaptor.h
)
if(LIBUDEV_MONITOR)
    list(APPEND lxqt-session_SRCS src/UdevNotifier.cpp)
endif()

set(lxqt-session_UI
    src/wmselectdialog.ui
)

# Translations
lxqt_translate_ts(lxqt-session_QM_FILES
    UPDATE_TRANSLATIONS
        ${UPDATE_TRANSLATIONS}
    SOURCES
        ${lxqt-session_HDRS}
        ${lxqt-session_SRCS}
        ${lxqt-session_UI}
    INSTALL_DIR
        "${LXQT_TRANSLATIONS_DIR}/${PROJECT_NAME}"
)
lxqt_app_translation_loader(lxqt-session_QM_LOADER ${PROJECT_NAME})

qt5_wrap_ui(UIS ${lxqt-session_UI})

add_executable(lxqt-session
    ${lxqt-session_SRCS}
    ${UIS}
    ${RSCS}
    ${lxqt-session_QM_FILES}
    ${lxqt-session_QM_LOADER}
)

target_link_libraries(lxqt-session
    Qt5::Widgets
    Qt5::DBus
    Qt5::X11Extras
    ${XCB_LIBRARIES}
    ${X11_LIBRARIES}
    lxqt
    KF5::WindowSystem
)
if(LIBUDEV_MONITOR)
    target_link_libraries(lxqt-session ${LIBUDEV_LIBRARIES})
endif()

INSTALL(TARGETS lxqt-session RUNTIME DESTINATION bin)
