cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

project(lxqt-common)

find_package(lxqt REQUIRED)

include(GNUInstallDirs)
include(LXQtTranslateDesktop)

install(DIRECTORY graphics DESTINATION "${LXQT_SHARE_DIR}")

# startlxqt script
configure_file(startlxqt.in startlxqt @ONLY)
install(PROGRAMS
    "${CMAKE_CURRENT_BINARY_DIR}/startlxqt"
    DESTINATION "${CMAKE_INSTALL_BINDIR}"
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
