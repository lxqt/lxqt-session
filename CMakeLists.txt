cmake_minimum_required(VERSION 2.8.11)

project(lxqt-common)

find_package(lxqt-qt5 REQUIRED)
include(${LXQT_USE_FILE})

include(LXQtTranslateDesktop)

# graphics
install(DIRECTORY graphics DESTINATION share/lxqt/)

# startlxqt script
configure_file(startlxqt.in startlxqt @ONLY)
install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/startlxqt" DESTINATION bin)

# default config files
add_subdirectory(config)
add_subdirectory(autostart)

# xsession *.desktop file for display managers
add_subdirectory(xsession)

# themes
add_subdirectory(themes)

# building tarball with CPack -------------------------------------------------
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_VERSION_MAJOR ${LXQT_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${LXQT_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${LXQT_PATCH_VERSION})
set(CPACK_GENERATOR TBZ2)
set(CPACK_SOURCE_GENERATOR TBZ2)
set(CPACK_SOURCE_IGNORE_FILES /build/;.gitignore;.*~;.git;.kdev4;temp)
include(CPack)
