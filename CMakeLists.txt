cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

project(lxqt-common)

find_package(lxqt REQUIRED)

include(GNUInstallDirs)
include(LXQtTranslateDesktop)

install(DIRECTORY "graphics/" DESTINATION "${LXQT_GRAPHICS_DIR}")

# xmessage dependency - to notify packagers
find_program(XMESSAGE xmessage PATHS ENV PATH NO_DEFAULT_PATH)
if (NOT XMESSAGE)
    message(SEND_ERROR "The run-time dependency of startlxt -> 'xmessage' is not found")
endif()

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

# building tarball with CPack -------------------------------------------------
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_VERSION_MAJOR ${LXQT_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${LXQT_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${LXQT_PATCH_VERSION})
set(CPACK_GENERATOR TBZ2)
set(CPACK_SOURCE_GENERATOR TBZ2)
set(CPACK_SOURCE_IGNORE_FILES /build/;.gitignore;.*~;.git;.kdev4;temp)
include(CPack)
