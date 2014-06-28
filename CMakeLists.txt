# install read-only data files and default config files for LXDE-Qt
cmake_minimum_required(VERSION 2.6)
project(lxqt-common)

# additional cmake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Support Qt5
option(USE_QT5 "Build with Qt5." $ENV{USE_QT5})
if(USE_QT5)
    find_package(lxqt-qt5 REQUIRED)
else()
    find_package(LXQT REQUIRED)
endif()
include(${LXQT_USE_FILE})

# graphics
install(DIRECTORY graphics DESTINATION share/lxqt/)

# startlxqt script
configure_file(startlxqt.in startlxqt @ONLY)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/startlxqt DESTINATION bin)

# default config files
add_subdirectory(config)
add_subdirectory(autostart)

# xsession *.desktop file for display managers
add_subdirectory(xsession)

# themes
add_subdirectory(themes)

# building tarball with CPack -------------------------------------------------
include (InstallRequiredSystemLibraries)
set (CPACK_PACKAGE_VERSION_MAJOR ${LXQT_MAJOR_VERSION})
set (CPACK_PACKAGE_VERSION_MINOR ${LXQT_MINOR_VERSION})
set (CPACK_PACKAGE_VERSION_PATCH ${LXQT_PATCH_VERSION})
set (CPACK_GENERATOR TBZ2)
set (CPACK_SOURCE_GENERATOR TBZ2)
set (CPACK_SOURCE_IGNORE_FILES /build/;.gitignore;.*~;.git;.kdev4;temp)
include (CPack)
