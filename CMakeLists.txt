cmake_minimum_required(VERSION 2.8.9)

project(lxqt-session)

# additional cmake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Support different versions of Qt
option(USE_QT5 "Force use the Qt5." $ENV{USE_QT5})
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_AUTOMOC ON)
if(USE_QT5)
    cmake_minimum_required(VERSION 2.8.11)
    find_package(Qt5Widgets REQUIRED QUIET)
    find_package(Qt5DBus REQUIRED QUIET)
    find_package(Qt5X11Extras REQUIRED QUIET)
    find_package(Qt5LinguistTools REQUIRED QUIET)

    find_package(lxqt-qt5 REQUIRED QUIET)
    include(${LXQT_USE_FILE})

    find_package(Qt5Xdg REQUIRED QUIET)

	find_package(PkgConfig REQUIRED QUIET)
	pkg_check_modules(XCB REQUIRED xcb)
else()
	find_package(Qt4 REQUIRED)
	include(${QT_USE_FILE})

	find_package(LXQT REQUIRED)
	include(${LXQT_USE_FILE})

    find_package(qtxdg REQUIRED QUIET)
endif()

find_package ( X11 REQUIRED )

add_subdirectory(lxqt-session)
add_subdirectory(lxqt-config-session)

# building tarball with CPack -------------------------------------------------
include (InstallRequiredSystemLibraries)
set (CPACK_PACKAGE_VERSION_MAJOR ${LXQT_MAJOR_VERSION})
set (CPACK_PACKAGE_VERSION_MINOR ${LXQT_MINOR_VERSION})
set (CPACK_PACKAGE_VERSION_PATCH ${LXQT_PATCH_VERSION})
set (CPACK_GENERATOR TBZ2)
set (CPACK_SOURCE_GENERATOR TBZ2)
set (CPACK_SOURCE_IGNORE_FILES /build/;.gitignore;.*~;.git;.kdev4;temp)
include (CPack)
