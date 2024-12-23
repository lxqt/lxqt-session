cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)
# CMP0000: Call the cmake_minimum_required() command at the beginning of the top-level
# CMakeLists.txt file even before calling the project() command.
# The cmake_minimum_required(VERSION) command implicitly invokes the cmake_policy(VERSION)
# command to specify that the current project code is written for the given range of CMake
# versions.
project(lxqt-session)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(GNUInstallDirs)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(UPDATE_TRANSLATIONS "Update source translation translations/*.ts files" OFF)
option(WITH_LIBUDEV "Build with libudev support" ON)

# Minimum Versions
set(KF6_MINIMUM_VERSION "6.0.0")
set(LXQT_MINIMUM_VERSION "2.1.0")
set(QTXDG_MINIMUM_VERSION "4.1.0")
set(QT_MINIMUM_VERSION "6.6.0")
set(SHELLQT_MINIMUM_VERSION "6.0.0")

find_package(Qt6DBus ${QT_MINIMUM_VERSION} REQUIRED)
find_package(Qt6LinguistTools ${QT_MINIMUM_VERSION} REQUIRED)
find_package(Qt6Widgets ${QT_MINIMUM_VERSION} REQUIRED)
find_package(KF6WindowSystem ${KF6_MINIMUM_VERSION} REQUIRED)
find_package(lxqt ${LXQT_MINIMUM_VERSION} REQUIRED)
find_package(qtxdg-tools ${QTXDG_MINIMUM_VERSION} REQUIRED)
find_package(X11 REQUIRED)
find_package(LayerShellQt ${SHELLQT_MINIMUM_VERSION} REQUIRED)
message(STATUS "Building with Qt${Qt6Core_VERSION}")
find_package(PkgConfig REQUIRED)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    pkg_search_module(PROCPS REQUIRED libproc2 libprocps)
    message(STATUS "Using PROCPS -> ${PROCPS_MODULE_NAME} v${PROCPS_VERSION}")
    if (PROCPS_VERSION VERSION_GREATER_EQUAL 4.0.0)
        add_definitions("-DUSING_LIBPROC2")
        if (PROCPS_VERSION VERSION_GREATER_EQUAL 4.0.5)
            add_definitions("-DUSING_LIBPROC2_1")
        endif()
    endif()
endif()

# Please don't move, must be after lxqt
find_package(XdgUserDirs REQUIRED)

# Patch Version
set(LXQT_SESSION_PATCH_VERSION 0)

set(LXQT_SESSION_VERSION ${LXQT_MAJOR_VERSION}.${LXQT_MINOR_VERSION}.${LXQT_SESSION_PATCH_VERSION})
add_definitions(
    "-DLXQT_SESSION_VERSION=\"${LXQT_SESSION_VERSION}\""
)

include(LXQtPreventInSourceBuilds)
include(LXQtCompilerSettings NO_POLICY_SCOPE)

# Translations **********************************
include(LXQtTranslate)

add_subdirectory(lxqt-session)
add_subdirectory(lxqt-config-session)
add_subdirectory(lxqt-leave)

# merged from lxqt-common
include(LXQtConfigVars)

# startlxqt script
set(PREDEF_XDG_DATA_DIRS "$XDG_DATA_HOME")
if(NOT("${LXQT_DATA_DIR}" MATCHES "^/usr(/local)?/share$"))
    set(PREDEF_XDG_DATA_DIRS "${PREDEF_XDG_DATA_DIRS}:${LXQT_DATA_DIR}")
endif()
set(PREDEF_XDG_DATA_DIRS "${PREDEF_XDG_DATA_DIRS}:/usr/local/share:/usr/share")
set(PREDEF_XDG_CONFIG_DIRS "/etc:${LXQT_ETC_XDG_DIR}:/usr/share")
configure_file(startlxqt.in startlxqt @ONLY)
install(PROGRAMS
    "${CMAKE_CURRENT_BINARY_DIR}/startlxqt"
    DESTINATION "${CMAKE_INSTALL_BINDIR}"
    COMPONENT Runtime
)
install(FILES
    startlxqt.1
    DESTINATION "${CMAKE_INSTALL_MANDIR}/man1"
    COMPONENT Runtime
)

# default config files
add_subdirectory(config)
add_subdirectory(autostart)

# xsession *.desktop file for display managers
add_subdirectory(xsession)

