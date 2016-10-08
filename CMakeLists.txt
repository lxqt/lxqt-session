cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)
project(lxqt-session)

option(UPDATE_TRANSLATIONS "Update source translation translations/*.ts files" OFF)
option(WITH_LIBUDEV "Build with libudev support" ON)

include(GNUInstallDirs)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


find_package(KF5WindowSystem REQUIRED)
find_package(Qt5Widgets REQUIRED QUIET)
find_package(Qt5DBus REQUIRED QUIET)
find_package(Qt5X11Extras REQUIRED QUIET)
find_package(Qt5LinguistTools REQUIRED QUIET)
find_package(lxqt-build-tools ${LXQTBT_MINIMUM_VERSION} REQUIRED)
find_package(lxqt REQUIRED QUIET)
find_package(XdgUserDirs REQUIRED QUIET)

include(LXQtCompilerSettings NO_POLICY_SCOPE)
include(LXQtTranslate)

find_package(PkgConfig REQUIRED QUIET)
pkg_check_modules(XCB REQUIRED xcb)
find_package(X11 REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_subdirectory(lxqt-session)
add_subdirectory(lxqt-config-session)
add_subdirectory(lxqt-leave)
