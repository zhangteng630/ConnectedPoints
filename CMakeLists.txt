cmake_minimum_required(VERSION 2.8)
project(ConnectedPoints)

set(VTK_DIR C:/libraries/VTK-build)
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

aux_source_directory(. SRC_LIST)
add_executable(connected_points ${SRC_LIST})
target_link_libraries(connected_points ${VTK_LIBRARIES})