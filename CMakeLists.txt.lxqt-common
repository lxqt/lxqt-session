cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

project(lxqt-common)

set(LXQTBT_MINIMUM_VERSION "0.1.0")

find_package(lxqt-build-tools ${LXQTBT_MINIMUM_VERSION} REQUIRED)

include(GNUInstallDirs)
include(LXQtTranslateDesktop)
include(LXQtConfigVars)

install(DIRECTORY "graphics/" DESTINATION "${LXQT_GRAPHICS_DIR}")

# startlxqt script
set(PREDEF_XDG_DATA_DIRS "$XDG_DATA_HOME")
if(NOT("${LXQT_DATA_DIR}" MATCHES "^/usr(/local)?/share$"))
    set(PREDEF_XDG_DATA_DIRS "${PREDEF_XDG_DATA_DIRS}:${LXQT_DATA_DIR}")
endif()
set(PREDEF_XDG_DATA_DIRS "${PREDEF_XDG_DATA_DIRS}:/usr/local/share:/usr/share")
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
add_subdirectory(themes)
add_subdirectory(menu)
add_subdirectory(icons)
