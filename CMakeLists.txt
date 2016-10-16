cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

project(lxqt-session)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(GNUInstallDirs)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(UPDATE_TRANSLATIONS "Update source translation translations/*.ts files" OFF)
option(WITH_LIBUDEV "Build with libudev support" ON)

find_package(PkgConfig REQUIRED QUIET)
find_package(X11 REQUIRED)
pkg_check_modules(XCB REQUIRED xcb)

find_package(Qt5DBus REQUIRED)
find_package(Qt5LinguistTools REQUIRED QUIET)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5X11Extras REQUIRED)
message(STATUS "Building with Qt${Qt5Core_VERSION_STRING}")

find_package(KF5WindowSystem REQUIRED)

find_package(lxqt REQUIRED)

# Please don't move, must be after lxqt
find_package(XdgUserDirs REQUIRED)

include(LXQtCompilerSettings NO_POLICY_SCOPE)

# Translations **********************************
include(LXQtTranslate)

add_subdirectory(lxqt-session)
add_subdirectory(lxqt-config-session)
add_subdirectory(lxqt-leave)
