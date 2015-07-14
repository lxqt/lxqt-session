cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

project(lxqt-session)

option(UPDATE_TRANSLATIONS "Update source translation translations/*.ts files" OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

include(GNUInstallDirs)
include(CheckCXXCompilerFlag)

CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(FATAL "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. C++11 support is required")
endif()

find_package(KF5WindowSystem REQUIRED)
find_package(Qt5Widgets REQUIRED QUIET)
find_package(Qt5DBus REQUIRED QUIET)
find_package(Qt5X11Extras REQUIRED QUIET)
find_package(Qt5LinguistTools REQUIRED QUIET)

find_package(lxqt REQUIRED QUIET)

include(LXQtTranslate)

find_package(PkgConfig REQUIRED QUIET)
pkg_check_modules(XCB REQUIRED xcb)
find_package(X11 REQUIRED)

add_subdirectory(lxqt-session)
add_subdirectory(lxqt-config-session)
add_subdirectory(lxqt-leave)

# building tarball with CPack -------------------------------------------------
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_VERSION_MAJOR ${LXQT_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${LXQT_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${LXQT_PATCH_VERSION})
set(CPACK_GENERATOR TBZ2)
set(CPACK_SOURCE_GENERATOR TBZ2)
set(CPACK_SOURCE_IGNORE_FILES /build/;.gitignore;.*~;.git;.kdev4;temp)
include(CPack)

