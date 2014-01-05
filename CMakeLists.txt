cmake_minimum_required(VERSION 2.6)

# additional cmake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(LXQT REQUIRED)
include(${LXQT_USE_FILE})

add_subdirectory(lxqt-session)
add_subdirectory(lxqt-config-session)


