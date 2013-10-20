# install read-only data files and default config files for LXDE-Qt
cmake_minimum_required(VERSION 2.6)
project(lxqt-common)

find_package(LXQT REQUIRED)
include(${LXQT_USE_FILE})

# graphics
install(DIRECTORY graphics DESTINATION share/lxqt/)

# startlxde-qt script
configure_file(startlxde-qt.in startlxde-qt @ONLY)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/startlxde-qt DESTINATION bin)

# default config files
add_subdirectory(config)
# add_subdirectory(autostart)

# xsession *.desktop file for display managers
add_subdirectory(xsession)

# themes
add_subdirectory(themes)
